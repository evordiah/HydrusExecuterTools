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

#include <thread>
#include <future>
#include <fstream>
#include <sstream>
#include <atomic>
#include "process.h"
#include "hydrusexcuternode.h"
#include "tcp_client_inner.h"
#include "applicationparametermanager.h"
#include "taskcontroller.h"

bool HydrusExecuterNode::GetId(int &gid)
{
    int iCount = 0;
    while (true)
    {
        switch (tcp_client_inner::enumTcpSocketState(GetTaskidFromServer(gid)))
        {
        case tcp_client_inner::enumTcpSocketState::Succeed:
            return true;
        case tcp_client_inner::enumTcpSocketState::Failed:
            if (gid != 0)
            {
                std::this_thread::sleep_for(std::chrono::seconds(gid));
            }
            else
            {
                return false;
            }
            break;
        default:
            if (++iCount > 9)
            {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            break;
        }
    }
}

bool HydrusExecuterNode::PrepareParameterFile(int gid)
{
    if (_pqry)
    {
        //std::string tmppath = std::filesystem::absolute(_currentprocesspath).native();
        std::string tmppath = std::filesystem::absolute(_currentprocesspath).string();
        _HydrusFilesManager = std::make_unique<HydrusParameterFilesManager>(gid, tmppath, *_pqry);
        bool result = _HydrusFilesManager->ExportInputFiles();
        if (!result)
        {
            std::string err("Can not get valid parameters");
            EnterFail(gid, err);
        }
        return result;
    }
    int iCount = 0;
    while (true)
    {
        switch (tcp_client_inner::enumTcpSocketState(GetParameterFromServer(gid)))
        {
        case tcp_client_inner::enumTcpSocketState::Succeed:
            return CreateHydrusFilesManager(gid);
        case tcp_client_inner::enumTcpSocketState::Failed:
            return false;
        default:
            if (++iCount > 9)
            {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            break;
        }
    }
}

bool HydrusExecuterNode::ExecuteHydrus(int gid)
{
    bool result = true;
    //std::string exefile = std::filesystem::absolute(_exepath / "hydrus.exe").native();
    //std::string pcpath = std::filesystem::absolute(_currentprocesspath).native();
    std::string exefile = std::filesystem::absolute(_exepath / "hydrus.exe").string();
    std::string pcpath = std::filesystem::absolute(_currentprocesspath).string();
    std::list<std::string> parms;
    std::string cmd;
    std::string winepath = ApplicationParameterManager::Instance().GetProgramPath("wine");
    if (winepath.empty())
    {
        cmd = exefile;
    }
    else
    {
        cmd = winepath;
        parms.push_back(exefile);
    }
    parms.push_back(pcpath);
    unsigned int msecs = ApplicationParameterManager::Instance().WaitSeconds();
    Process p(cmd, parms);
    p.WaitSeconds(msecs);
    std::condition_variable cond;
    std::atomic_bool _bEnterRunningOK(true);
    std::atomic_bool _StateThreadFinished(false);
    std::atomic_bool _ProcThreadFinished(false);
    auto st = [&] {
        _bEnterRunningOK.store(EnterRunning(gid));
        _StateThreadFinished.store(true);
        cond.notify_one();
    };
    auto pt = [&] {
        p.Run();
        _ProcThreadFinished.store(true);
        cond.notify_one();
    };
    std::thread(st).detach();
    std::thread(pt).detach();
    {
        std::mutex mux;
        std::unique_lock<std::mutex> ul(mux);
        cond.wait(ul, [&] { return _StateThreadFinished.load(); });
        if (!_bEnterRunningOK.load() && !_ProcThreadFinished.load())
        {
            p.Terminate();
            _ProcThreadFinished.store(true);
        }
        if (!_ProcThreadFinished.load())
        {
            cond.wait(ul, [&] { return _ProcThreadFinished.load(); });
        }
    }
    if (!_bEnterRunningOK.load())
    {
        return false;
    }
    std::string reason;
    switch (p.State())
    {
    case Process::state::FailedToStart:
        reason = "hydrus Failed to start";
        EnterFail(gid, reason);
        result = false;
        break;
    case Process::state::Crashed:
    case Process::state::WriteError:
    case Process::state::ReadError:
        reason = "hydrus Failed to execute";
        EnterFail(gid, reason);
        result = false;
        break;
    case Process::state::Timedout:
        EnterTimeout(gid);
        result = false;
        break;
    default:
        break;
    }
    return result;
}

bool HydrusExecuterNode::EnterRunning(int gid)
{
    int iCount = 0;
    while (true)
    {
        switch (tcp_client_inner::enumTcpSocketState(SetRunningState(gid)))
        {
        case tcp_client_inner::enumTcpSocketState::Succeed:
            return true;
        case tcp_client_inner::enumTcpSocketState::Failed:
            return false;
        default:
            if (++iCount > 9)
            {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            break;
        }
    }
}

bool HydrusExecuterNode::EnterFail(int gid, std::string &reason)
{
    return TaskController::GetController().RecordError(gid, _grpid, reason);
}

bool HydrusExecuterNode::EnterTimeout(int gid)
{
    std::string temp("hydrus timeout");
    return TaskController::GetController().RecordError(gid, _grpid, temp);
}

bool HydrusExecuterNode::EnterComplete(int gid, std::string &result)
{
    return TaskController::GetController().PushResult(gid, _grpid, result);
}

// bool HydrusExecuterNode::EnterComplete(int gid, const std::string &result)
// {
//    std::stringstream  strbld;
//    strbld<<gid<<".RES";
//    std::filesystem::path p=std::filesystem::absolute(_exepath / strbld.str());
//    std::string filename=p.native();
//    std::ofstream out(filename);
//    if(!out)
//    {
//        return false;
//    }
//    out<<result;
//    out.close();
//    TaskController::GetController().PushResult(gid,_grpid,filename);
//    return true;
// }

int HydrusExecuterNode::GetTaskidFromServer(int &gid)
{
    //get taskid and grpid from server
    asio::io_context io;
    auto tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::requireTaskID);
    const std::string &ip = ApplicationParameterManager::Instance().HostIP();
    unsigned short port = ApplicationParameterManager::Instance().HostPort();
    tcp->GroupID(_grpid);
    tcp->TaskID(gid);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
    tcp->start(ep);
    io.run();
    gid = tcp->TaskID();
    auto tgrpid = tcp->GroupID();
    std::string strstate = tcp->UserData();
    if (tgrpid != _grpid)
    {
        _grpid = tgrpid;
    }
    auto s = tcp->State();
    int result = int(s);
    switch (s)
    {
    case tcp_client_inner::enumTcpSocketState::Succeed:
        if (strstate != "normal")
        {
            result = int(tcp_client_inner::enumTcpSocketState::Failed);
        }
        break;
    default:
        break;
    }
    return result;
}

int HydrusExecuterNode::GetParameterFromServer(int gid)
{
    asio::io_context io;
    auto tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::requireParameter);
    const std::string &ip = ApplicationParameterManager::Instance().HostIP();
    unsigned short port = ApplicationParameterManager::Instance().HostPort();
    tcp->GroupID(_grpid);
    tcp->TaskID(gid);
    std::string path = std::filesystem::absolute(_currentprocesspath).string();
    tcp->UserData(path);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
    tcp->start(ep);
    io.run();
    return int(tcp->State());
}

int HydrusExecuterNode::SetRunningState(int gid)
{
    asio::io_context io;
    auto tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::notifyRunning);
    const std::string &ip = ApplicationParameterManager::Instance().HostIP();
    unsigned short port = ApplicationParameterManager::Instance().HostPort();
    tcp->GroupID(_grpid);
    tcp->TaskID(gid);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
    tcp->start(ep);
    io.run();
    return int(tcp->State());
}

bool HydrusExecuterNode::CreateHydrusFilesManager(int gid)
{
    unsigned int nlayer, ns, nobs;
    std::string status, line;
    std::unique_ptr<unsigned int[]> pobs;
    auto p = std::filesystem::absolute(_currentprocesspath / "PARAMETERS.TMP").string();
    std::ifstream in(p);
    if (!in)
    {
        return false;
    }
    std::getline(in, status);
    std::getline(in, line);
    std::stringstream strin;
    strin.str(line);
    strin >> nlayer >> ns >> nobs;
    if (nobs)
    {
        pobs.reset(new unsigned int[nobs]);
        for (unsigned int i = 0; i < nobs; ++i)
        {
            strin >> pobs[i];
        }
    }
    in.close();
    //std::string tmppath = std::filesystem::absolute(_currentprocesspath).native();
    std::string tmppath = std::filesystem::absolute(_currentprocesspath).string();
    _HydrusFilesManager.reset(new HydrusParameterFilesManager(gid, tmppath, nlayer, ns, nobs, pobs.get(), status));
    return true;
}
