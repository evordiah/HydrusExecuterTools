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
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <future>
#include <regex>
#include <iostream>
#include "asio.hpp"
#include "servertaskcontroller.h"
#include "applicationparametermanager.h"
#include "hydrusexcuterserver.h"
#ifdef ABSTRACTJOB
#include "serverjob.h"
#elif defined COMMITJOB
#include "commitjob.h"
#include "asyncresultcontainer_server.h"
#endif

bool ServerTaskController::NextTaskID(int &id, unsigned int &grpid)
{
    if (!grpid)
    {
        grpid = ApplicationParameterManager::Instance().NewGroupID();
    }
    return _taskContainer.GetFreeTaskId(id, grpid);
}

bool ServerTaskController::PushResult(int gid, unsigned int grpid, std::string &result)
{
    if (!TaskController::GetController().SetCompleteState(gid, grpid))
    {
        return false;
    }
#ifdef COMMITJOB
    if (result.empty())
    {
        _resultContainer->Commit(CommitJob(gid, grpid));
    }
    else
    {
        _resultContainer->Commit(CommitJob(gid, grpid, result));
    }
#elif defined ABSTRACTJOB
    if (result.empty())
    {
        _resultContainer->Commit(std::unique_ptr<AbstractJob>(new ServerJob(gid, grpid)));
    }
    else
    {
        _resultContainer->Commit(std::unique_ptr<AbstractJob>(new ServerJob(gid, grpid, result)));
    }
#endif
    return true;
}

bool ServerTaskController::RecordError(int gid, unsigned int grpid, std::string &err)
{
    bool stateSucc = false;
    if (std::regex_search(err, std::regex("timeout", std::regex::icase)))
    {
        stateSucc = TaskController::GetController().SetTimeoutState(gid, grpid);
    }
    else
    {
        stateSucc = TaskController::GetController().SetFailState(gid, grpid);
    }
    if (stateSucc)
    {
#ifdef COMMITJOB
        if (err.empty())
        {
            _resultContainer->Commit(CommitJob(gid, grpid));
        }
        else
        {
            _resultContainer->Commit(CommitJob(gid, grpid, err, true));
        }
#elif defined ABSTRACTJOB
        if (err.empty())
        {
            _resultContainer->Commit(std::unique_ptr<AbstractJob>(new ServerJob(gid, grpid)));
        }
        else
        {
            _resultContainer->Commit(std::unique_ptr<AbstractJob>(new ServerJob(gid, grpid, err, true)));
        }
#endif
    }
    return stateSucc;
}

void ServerTaskController::Run()
{
    asio::io_context io;
    Run(io);
}

void ServerTaskController::Run(asio::io_context &io)
{
    size_t taskcnt = TaskCount();
    if (taskcnt <= 0)
    {
        std::cout << "\n*********There no valid gids. Exit now.*********\n\n";
        return;
    }
    _resultContainer = ResultsContainer_Server::Instance(io, taskcnt);

    std::vector<std::shared_ptr<HydrusExecuter>> _executers;
    std::vector<std::future<void>> _futures;
    std::shared_ptr<HydrusExecuter> pHydrusExe;
    unsigned int TCnt = ApplicationParameterManager::Instance().ThreadCount();
    for (unsigned int i = 0; i < TCnt; ++i)
    {
        pHydrusExe.reset(new HydrusExecuterServer());
        _executers.push_back(pHydrusExe);
        _futures.push_back(std::async(std::launch::async, &HydrusExecuter::Execute, _executers[i]));
    }
    io.run();
    _futures.clear();
    _executers.clear();
    std::filesystem::path qexepath = ApplicationParameterManager::Instance().HydursExecutingPath();
    std::filesystem::remove_all(qexepath);
}
