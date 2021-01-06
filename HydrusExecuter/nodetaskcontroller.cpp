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
#include <future>
#include <filesystem>
#include "asio.hpp"
#include "hydrusexcuternode.h"
#include "nodetaskcontroller.h"
#include "applicationparametermanager.h"
#ifdef ABSTRACTJOB
#include "nodejob.h"
#elif defined COMMITJOB
#include "commitjob.h"
#include "asyncresultcontainer_node.h"
#endif

bool NodeTaskController::HasNoTask()
{
    return 0 == ApplicationParameterManager::RunningThreadCount();
}

bool NodeTaskController::PushResult(int gid, unsigned int grpid, std::string &result)
{
#ifdef COMMITJOB
    _resultContainer->Commit(CommitJob(gid, grpid, result));
#elif defined ABSTRACTJOB
    _resultContainer->Commit(std::unique_ptr<AbstractJob>(new NodeJob(gid, grpid, result)));
#endif
    return true;
}

bool NodeTaskController::RecordError(int gid, unsigned int grpid, std::string &err)
{
#ifdef COMMITJOB
    _resultContainer->Commit(CommitJob(gid, grpid, err, true));
#elif defined ABSTRACTJOB
    _resultContainer->Commit(std::unique_ptr<AbstractJob>(new NodeJob(gid, grpid, err, true)));
#endif
    return true;
}

void NodeTaskController::Run()
{
    asio::io_context io;
    Run(io);
}

void NodeTaskController::Run(asio::io_context &io)
{
    _resultContainer = ResultsContainer_Node::Instance(io);
    std::vector<std::shared_ptr<HydrusExecuter>> _executers;
    std::vector<std::future<void>> _futures;
    std::shared_ptr<HydrusExecuter> pHydrusExe;
    unsigned int TCnt = ApplicationParameterManager::Instance().ThreadCount();
    for (unsigned int i = 0; i < TCnt; ++i)
    {
        pHydrusExe.reset(new HydrusExecuterNode());
        _executers.push_back(pHydrusExe);
        _futures.push_back(std::async(std::launch::async, &HydrusExecuter::Execute, _executers[i]));
    }
    //====================test===========
    // {
    //     pHydrusExe.reset(new HydrusExecuterNode());
    //     pHydrusExe->Execute();
    // }
    //====================test over=======
    io.run();
    _futures.clear();
    _executers.clear();
    std::filesystem::path qexepath = ApplicationParameterManager::Instance().HydursExecutingPath();
    std::filesystem::remove_all(qexepath);
}
