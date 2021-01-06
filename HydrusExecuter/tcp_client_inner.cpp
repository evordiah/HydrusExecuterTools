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

#include <fstream>
#include <sstream>
#include <filesystem>
#include "tcp_client_inner.h"
#include "tinyxml2/tinyxml2.h"
#include "b64/decode.h"

tcp_client_inner::Tcp_Client::Tcp_Client(asio::io_context &io, tcp_client_inner::enumRequire enumr)
    : _socket(io), _started(false), _State(enumTcpSocketState::Failed), _sendbuf(""), _revbuf("")
{
    switch (enumr)
    {
    case enumRequire::requireConnectionString:
        _pRequirer.reset(new ConnectionStringRequirer());
        break;
    case enumRequire::requireTaskID:
        _pRequirer.reset(new TaskIDRequirer());
        break;
    case enumRequire::requireParameter:
        _pRequirer.reset(new ParameterRequirer());
        break;
    case enumRequire::requireHydrus:
        _pRequirer.reset(new HydrusRequier());
        break;
    case enumRequire::notifyRunning:
        _pRequirer.reset(new RunningNotify());
        break;
    case enumRequire::notifyFail:
        _pRequirer.reset(new FailedNotify());
        break;
    case enumRequire::notifyTimeout:
        _pRequirer.reset(new TimeoutNotify());
        break;
    case enumRequire::notifyComplete:
        _pRequirer.reset(new CompleteNotify());
        break;
    }
}

void tcp_client_inner::Tcp_Client::OnRead(const asio::error_code &err, size_t bytes)
{
    if (!err && _pRequirer->Receive(_revbuf))
    {
        _State = enumTcpSocketState::Succeed;
    }
    else
    {
        _State = enumTcpSocketState::Failed;
    }
    stop();
}

void tcp_client_inner::Tcp_Client::OnWrite(const asio::error_code &err, size_t bytes)
{
    if (!err)
    {
        asio::async_read_until(_socket, asio::dynamic_buffer(_revbuf), "</message>", std::bind(&Tcp_Client::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        _State = enumTcpSocketState::FatalError;
        stop();
    }
}

void tcp_client_inner::Tcp_Client::onConnect(const asio::error_code &err)
{
    if (!err && _pRequirer->Require(_sendbuf))
    {
        asio::async_write(_socket, asio::buffer(_sendbuf), std::bind(&Tcp_Client::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        _State = enumTcpSocketState::FatalError;
        stop();
    }
}

bool tcp_client_inner::BaseRequirer::Require(std::string &require)
{
    if (PrePareData())
    {
        require = std::move(_senddata);
        return true;
    }
    return false;
}

bool tcp_client_inner::BaseRequirer::Receive(std::string &answer)
{
    if (!answer.empty())
    {
        _revdata = std::move(answer);
        auto pos = _revdata.rfind("</message>");
        if (pos != std::string::npos)
        {
            _revdata.erase(pos + 10);
            return ParseAnswerData();
        }
    }
    return false;
}

bool tcp_client_inner::ConnectionStringRequirer::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "ConnectionString");
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::ConnectionStringRequirer::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            const char *pValue = pContent->Attribute("value");
            if (pValue)
            {
                _revdata.clear();
                _revdata.append(pValue);
            }
            else
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        _revdata.clear();
        result = false;
    }
    return result;
}

bool tcp_client_inner::TaskIDRequirer::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "TaskID");
    cele->SetAttribute("grpid", _grpid);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::TaskIDRequirer::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            bool Succeed = true;
            bool Normal = true;
            if (pContent->QueryAttribute("success", &Succeed) == tinyxml2::XML_SUCCESS)
            {
                if (pContent->QueryAttribute("grpid", &_grpid) != tinyxml2::XML_SUCCESS ||
                    pContent->QueryAttribute("taskid", &_taskid) != tinyxml2::XML_SUCCESS ||
                    pContent->QueryAttribute("state", &Normal) != tinyxml2::XML_SUCCESS ||
                    !Succeed )
                {
                    result = false;
                }
                else
                {
                    _userdata = Normal ? "normal" : "wait";
                }
            }
            else
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    return result;
}

bool tcp_client_inner::ParameterRequirer::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "Parameter");
    cele->SetAttribute("grpid", _grpid);
    cele->SetAttribute("taskid", _taskid);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::ParameterRequirer::ParseAnswerData()
{
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) != tinyxml2::XML_SUCCESS)
    {
        return false;
    }
    tinyxml2::XMLElement *pRoot = doc.RootElement();
    tinyxml2::XMLElement *pEle = pRoot->FirstChildElement("content");
    if (!pEle)
    {
        return false;
    }
    bool Succeed = true;
    if (pEle->QueryAttribute("success", &Succeed) != tinyxml2::XML_SUCCESS || !Succeed)
    {
        return false;
    }
    std::string status;
    const char *pcstatus = pEle->Attribute("status");
    if (!pcstatus)
    {
        return false;
    }
    status = pcstatus;
    unsigned int nlayer, ns, nobs;
    if (pEle->QueryAttribute("layer", &nlayer) != tinyxml2::XML_SUCCESS ||
        pEle->QueryAttribute("ns", &ns) != tinyxml2::XML_SUCCESS ||
        pEle->QueryAttribute("nobs", &nobs) != tinyxml2::XML_SUCCESS)
    {
        return false;
    }
    std::unique_ptr<unsigned int[]> pobs;
    if (nobs)
    {
        pobs.reset(new unsigned int[nobs]);
        const char *pstr = pEle->Attribute("obsnode");
        if (!pstr)
        {
            return false;
        }
        std::stringstream in(pstr);
        for (unsigned int i = 0; i < nobs; ++i)
        {
            in >> pobs[i];
        }
    }
    std::filesystem::path p = _userdata;
    if (!std::filesystem::exists(p))
    {
        if (!std::filesystem::create_directories(p))
        {
            return false;
        }
    }
    std::string strvalue;
    pEle = pRoot->FirstChildElement("selector");
    if (!pEle)
    {
        return false;
    }
    const char *pcValue = pEle->Attribute("value");
    if (!pcValue)
    {
        return false;
    }
    strvalue = pcValue;
    std::ofstream out((std::filesystem::absolute(p) / "SELECTOR.IN").string());
    if (!out)
    {
        return false;
    }
    out << strvalue;
    out.close();
    out.clear();
    pEle = pRoot->FirstChildElement("atmosph");
    if (!pEle)
    {
        return false;
    }
    pcValue = pEle->Attribute("value");
    if (!pcValue)
    {
        return false;
    }
    strvalue = pcValue;
    out.open((std::filesystem::absolute(p) / "ATMOSPH.IN").string());
    if (!out)
    {
        return false;
    }
    out << strvalue;
    out.close();
    out.clear();
    pEle = pRoot->FirstChildElement("profile");
    if (!pEle)
    {
        return false;
    }
    pcValue = pEle->Attribute("value");
    if (!pcValue)
    {
        return false;
    }
    strvalue = pcValue;
    out.open((std::filesystem::absolute(p) / "PROFILE.DAT").string());
    if (!out)
    {
        return false;
    }
    out << strvalue;
    out.close();
    out.clear();
    out.open((std::filesystem::absolute(p) / "PARAMETERS.TMP").string());
    if (!out)
    {
        return false;
    }
    out << status << std::endl;
    out << nlayer << "\t"
        << ns << "\t"
        << nobs;
    for (unsigned int i = 0; i < nobs; ++i)
    {
        out << "\t"
            << pobs[i];
    }
    out << std::endl;
    out.close();
    return true;
}

bool tcp_client_inner::CompleteNotify::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "NofityComplete");
    cele->SetAttribute("grpid", _grpid);
    cele->SetAttribute("taskid", _taskid);
    cele->SetAttribute("commit", _bCommitToDatabase);
    if (_bCommitToDatabase)
    {
        cele->SetAttribute("value", _senddata.c_str());
    }
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::CompleteNotify::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            if (pContent->QueryAttribute("success", &result) != tinyxml2::XML_SUCCESS)
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    return result;
}

bool tcp_client_inner::TimeoutNotify::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "NofityTimeout");
    cele->SetAttribute("grpid", _grpid);
    cele->SetAttribute("taskid", _taskid);
    cele->SetAttribute("commit", _bCommitToDatabase);
    if (_bCommitToDatabase)
    {
        cele->SetAttribute("value", _senddata.c_str());
    }
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::TimeoutNotify::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            if (pContent->QueryAttribute("success", &result) != tinyxml2::XML_SUCCESS)
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    return result;
}

bool tcp_client_inner::FailedNotify::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "NofityFail");
    cele->SetAttribute("grpid", _grpid);
    cele->SetAttribute("taskid", _taskid);
    cele->SetAttribute("commit", _bCommitToDatabase);
    if (_bCommitToDatabase)
    {
        cele->SetAttribute("value", _senddata.c_str());
    }
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::FailedNotify::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            if (pContent->QueryAttribute("success", &result) != tinyxml2::XML_SUCCESS)
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    return result;
}

bool tcp_client_inner::RunningNotify::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "NofityRun");
    cele->SetAttribute("grpid", _grpid);
    cele->SetAttribute("taskid", _taskid);
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::RunningNotify::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            if (pContent->QueryAttribute("success", &result) != tinyxml2::XML_SUCCESS)
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    return result;
}

bool tcp_client_inner::HydrusRequier::PrePareData()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("type", "Hydrus");
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    _senddata = "";
    _senddata.append(prt.CStr());
    return true;
}

bool tcp_client_inner::HydrusRequier::ParseAnswerData()
{
    bool result = true;
    tinyxml2::XMLDocument doc;
    if (doc.Parse(_revdata.c_str()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement *pRoot = doc.RootElement();
        tinyxml2::XMLElement *pContent = pRoot->FirstChildElement("content");
        if (pContent)
        {
            bool Succeed = true;
            if (pContent->QueryAttribute("success", &Succeed) == tinyxml2::XML_SUCCESS && Succeed)
            {
                const char *pValue = pContent->Attribute("value");
                if (pValue)
                {
                    std::stringstream instream(pValue);
                    std::ofstream outstream(_userdata.c_str(), std::ios_base::out | std::ios_base::binary);
                    if (outstream)
                    {
                        base64::decoder D;
                        D.decode(instream, outstream);
                    }
                    else
                    {
                        result = false;
                    }
                    outstream.close();
                }
                else
                {
                    result = false;
                }
            }
            else
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    return result;
}
