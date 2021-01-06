/******************************************************************************
 *
 *
 *  Copyright (c) 2020, Wenzhao Feng.
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include <functional>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "tinyxml2/tinyxml2.h"
#include "b64/encode.h"
#include "tcp_server_inner.h"
#include "pqdbconn.h"
#include "taskcontroller.h"
#include "HydrusParameterFilesManager.h"
#include "applicationparametermanager.h"
#include "hydrusresource.h"

void tcp_server_inner::TcpServer::do_Read()
{
    asio::async_read_until(_socket, asio::dynamic_buffer(_revbuf), "</message>", std::bind(&TcpServer::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void tcp_server_inner::TcpServer::OnRead(const asio::error_code &err, size_t bytes)
{
    if (!err)
    {
        auto pos = _revbuf.rfind("</message>");
        _revbuf.erase(pos + 10);
        tinyxml2::XMLDocument doc;
        if (doc.Parse(_revbuf.c_str()) == tinyxml2::XML_SUCCESS)
        {
            tinyxml2::XMLElement *pRoot = doc.RootElement();
            tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
            if (pContent)
            {
                const char *pValue = pContent->Attribute("type");
                if (pValue)
                {
                    CreateResponder(pValue, pContent);
                    do_Write();
                    return;
                }
            }
        }
    }
    _revbuf.clear();
    stop();
}

void tcp_server_inner::TcpServer::CreateResponder(const std::string &type, tinyxml2::XMLElement *pEle)
{
    if (type == "ConnectionString")
    {
        _pResponder.reset(new ConnectionStringResponder());
        return;
    }
    if (type == "TaskID")
    {
        _pResponder.reset(new TaskIDResponder(pEle));
        return;
    }
    if (type == "Parameter")
    {
        _pResponder.reset(new ParameterResponder(pEle));
        return;
    }
    if (type == "NofityRun")
    {
        _pResponder.reset(new RunningResponder(pEle));
        return;
    }
    if (type == "NofityFail")
    {
        _pResponder.reset(new FailedResponder(pEle));
        return;
    }
    if (type == "NofityTimeout")
    {
        _pResponder.reset(new TimeoutResponder(pEle));
        return;
    }
    if (type == "NofityComplete")
    {
        _pResponder.reset(new CompleteResponder(pEle));
        return;
    }
    if (type == "Hydrus")
    {
        _pResponder.reset(new HydrusResponder());
    }
}

void tcp_server_inner::TcpServer::do_Write()
{
    if (_pResponder->Respond(_sendbuf))
    {
        asio::async_write(_socket, asio::buffer(_sendbuf), std::bind(&TcpServer::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        return;
    }
    stop();
}

void tcp_server_inner::TcpServer::OnWrite(const asio::error_code &err, size_t bytes)
{
    stop();
}

bool tcp_server_inner::Responder::Respond(std::string &answer)
{
    if (!_respondData.empty())
    {
        answer = std::move(_respondData);
        return true;
    }
    return false;
}

tcp_server_inner::ConnectionStringResponder::ConnectionStringResponder()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("value", DBConnManager::GetInstance().GetConnection()->GetConnectionString().c_str());
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}

tcp_server_inner::TaskIDResponder::TaskIDResponder(tinyxml2::XMLElement *pCont)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    if (pCont->QueryAttribute("grpid", &_grpid) == tinyxml2::XML_SUCCESS)
    {
        bool result = TaskController::GetController().NextTaskID(_taskid, _grpid);
        cele->SetAttribute("grpid", _grpid);
        cele->SetAttribute("taskid", _taskid);
        if (!result && !_taskid)
        {
            cele->SetAttribute("success", false);
        }
        else
        {
            cele->SetAttribute("success", true);
            cele->SetAttribute("state", result);
        }
    }
    else
    {
        cele->SetAttribute("success", false);
    }
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}

tcp_server_inner::RunningResponder::RunningResponder(tinyxml2::XMLElement *pCont)
{
    bool result = false;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    if (pCont->QueryAttribute("grpid", &_grpid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("taskid", &_taskid) == tinyxml2::XML_SUCCESS)
    {
        result = TaskController::GetController().SetRunningState(_taskid, _grpid);
    }
    cele->SetAttribute("success", result);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}

tcp_server_inner::FailedResponder::FailedResponder(tinyxml2::XMLElement *pCont)
{
    bool result = false;
    bool bCommit = false;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    if (pCont->QueryAttribute("grpid", &_grpid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("taskid", &_taskid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("commit", &bCommit) == tinyxml2::XML_SUCCESS)
    {
        // if (TaskController::GetController().SetFailState(_taskid, _grpid))
        // {
        if (bCommit)
        {
            const char *pVal;
            std::string err = (pVal = pCont->Attribute("value")) ? pVal : "";
            result = TaskController::GetController().RecordError(_taskid, _grpid, err);
        }
        else
        {
            std::string err;
            result = TaskController::GetController().RecordError(_taskid, _grpid, err);
        }
        // }
    }
    cele->SetAttribute("success", result);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}

tcp_server_inner::TimeoutResponder::TimeoutResponder(tinyxml2::XMLElement *pCont)
{
    bool result = false;
    bool bCommit = false;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    if (pCont->QueryAttribute("grpid", &_grpid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("taskid", &_taskid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("commit", &bCommit) == tinyxml2::XML_SUCCESS)
    {
        // if (TaskController::GetController().SetTimeoutState(_taskid, _grpid))
        // {
        if (bCommit)
        {
            const char *pVal;
            std::string err = (pVal = pCont->Attribute("value")) ? pVal : "";
            result = TaskController::GetController().RecordError(_taskid, _grpid, err);
        }
        else
        {
            std::string err;
            result = TaskController::GetController().RecordError(_taskid, _grpid, err);
        }
        // }
    }
    cele->SetAttribute("success", result);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}

tcp_server_inner::CompleteResponder::CompleteResponder(tinyxml2::XMLElement *pCont)
{
    bool result = false;
    bool bCommit = false;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    if (pCont->QueryAttribute("grpid", &_grpid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("taskid", &_taskid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("commit", &bCommit) == tinyxml2::XML_SUCCESS)
    {
        // if (TaskController::GetController().SetCompleteState(_taskid, _grpid))
        // {
        if (bCommit)
        {
            const char *pVal;
            std::string res = (pVal = pCont->Attribute("value")) ? pVal : "";
            result = TaskController::GetController().PushResult(_taskid, _grpid, res);
        }
        else
        {
            std::string res;
            result = TaskController::GetController().PushResult(_taskid, _grpid, res);
        }
        // }
    }
    cele->SetAttribute("success", result);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}

tcp_server_inner::ParameterResponder::ParameterResponder(tinyxml2::XMLElement *pCont)
{
    bool result = false;
    if (pCont->QueryAttribute("grpid", &_grpid) == tinyxml2::XML_SUCCESS &&
        pCont->QueryAttribute("taskid", &_taskid) == tinyxml2::XML_SUCCESS)
    {
        if (TaskController::GetController().CheckLock(_taskid, _grpid))
        {
            pqDBConn *pConn = DBConnManager::GetInstance().GetorCloneConnection(std::to_string(_grpid));
            pqxx::connection *pqry = pConn ? pConn->GetConn() : nullptr;
            if (pqry)
            {
                HydrusParameterFilesManager manager(_taskid, *pqry);
                _respondData = "";
                if (manager.ExportInputFiles(_respondData))
                {

                    result = true;
                }
            }
            if (!result)
            {
                // if (TaskController::GetController().SetFailState(_taskid, _grpid))
                // {
                std::string tmp("Can not get valid parameters");
                TaskController::GetController().RecordError(_taskid, _grpid, tmp);
                // }
            }
        }
    }
    if (!result)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement *mele = doc.NewElement("message");
        tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
        cele->SetAttribute("success", false);
        doc.InsertFirstChild(mele);
        tinyxml2::XMLPrinter prt(0, true);
        doc.Accept(&prt);
        _respondData = "";
        _respondData.append(prt.CStr());
    }
}

tcp_server_inner::HydrusResponder::HydrusResponder()
{
    bool result = false;
    std::ostringstream out;
    base64::encoder e;
    std::filesystem::path p = ApplicationParameterManager::Instance().HydursExecutingPath();
    p /= "hydrus.exe";
    if (std::filesystem::exists(p))
    {
        std::string path = p.string();
        std::ifstream instream(path, std::ios_base::binary | std::ios_base::in);
        if (instream)
        {
            e.encode(instream, out);
            instream.close();
            result = true;
        }
    }
    if (!result)
    {
        std::stringstream instrm;
        instrm.write(reinterpret_cast<const char *>(hydrus_resource), hydrus_resource_len);
        e.encode(instrm, out);
    }
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("success", true);
    cele->SetAttribute("value", out.str().c_str());
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _respondData = "";
    _respondData.append(prt.CStr());
}
