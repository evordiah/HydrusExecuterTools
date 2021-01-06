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
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "process.h"
#include "hydrusexcuterserver.h"
#include "taskcontroller.h"
#include "applicationparametermanager.h"

bool HydrusExecuterServer::GetId(int &gid)
{
    while (!TaskController::GetController().HasNoTask())
    {
        if (TaskController::GetController().NextTaskID(gid, _grpid))
        {
            return true;
        }
        if (!gid)
        {
            gid = 5;
        }
        std::this_thread::sleep_for(std::chrono::seconds(gid));
    }
    return false;
}

bool HydrusExecuterServer::PrepareParameterFile(int gid)
{
    //std::string tmppath = std::filesystem::absolute(_currentprocesspath).native();
    std::string tmppath = std::filesystem::absolute(_currentprocesspath).string();
    _HydrusFilesManager = std::make_unique<HydrusParameterFilesManager>(gid, tmppath, *_pqry);
    bool result = _HydrusFilesManager->ExportInputFiles();
    if (!result)
    {
        std::string err("Can not get valid parameters");
        TaskController::GetController().RecordError(gid, _grpid, err);
        std::cerr << err << " for gid = " << gid << std::endl;
    }
    return result;
}

bool HydrusExecuterServer::ExecuteHydrus(int gid)
{
    bool result = true;
    //std::string exefile = std::filesystem::absolute(_exepath / "hydrus.exe").native();
    //std::string pcpath = std::filesystem::absolute(_currentprocesspath).native();
    std::string exefile = std::filesystem::absolute(_exepath / "hydrus.exe").string();
    std::string pcpath = std::filesystem::absolute(_currentprocesspath).string();
    std::list<std::string> parms;

    std::string winepath = ApplicationParameterManager::Instance().GetProgramPath("wine");
    std::string cmd;
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
    if (!EnterRunning(gid))
    {
        return false;
    }
    unsigned int msecs = ApplicationParameterManager::Instance().WaitSeconds();
    Process p(cmd, parms);
    p.WaitSeconds(msecs);
    bool runresult = p.Run();
    std::string reason;
    switch (p.State())
    {
    case Process::state::FailedToStart:
        reason = "hydrus Failed to start";
        EnterFail(gid, reason);
        result = false;
        break;
    case Process::state::WriteError:
    case Process::state::ReadError:
        if (!runresult)
        {
            reason = "hydrus Failed to execute with writeerror or readerror";
            EnterFail(gid, reason);
            result = false;
        }
        break;
    case Process::state::Crashed:
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

bool HydrusExecuterServer::EnterRunning(int gid)
{
    return TaskController::GetController().SetRunningState(gid, _grpid);
}

bool HydrusExecuterServer::EnterFail(int gid, std::string &reason)
{
    TaskController::GetController().RecordError(gid, _grpid, reason);
    return true;
}

bool HydrusExecuterServer::EnterTimeout(int gid)
{
    std::string err("hydrus timeout");
    TaskController::GetController().RecordError(gid, _grpid, err);
    return true;
}

bool HydrusExecuterServer::EnterComplete(int gid, std::string &result)
{
    TaskController::GetController().PushResult(gid, _grpid, result);
    return true;
}

//bool HydrusExecuterServer::EnterComplete(int gid, std::string &result)
//{
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
//}
