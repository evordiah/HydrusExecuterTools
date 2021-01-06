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

#ifndef RESULTSCONTAINER_SERVER_H
#define RESULTSCONTAINER_SERVER_H

#include <condition_variable>
#include "asyncresultcontainer.h"

class ResultsContainer_Server : ResultsContainer
{
public:
    ResultsContainer_Server(const ResultsContainer_Server &rhs) = delete;
    ResultsContainer_Server(ResultsContainer_Server &&rhs) = delete;
    ResultsContainer_Server &operator=(const ResultsContainer_Server &rhs) = delete;
    ResultsContainer_Server &operator=(ResultsContainer_Server &&rhs) = delete;

    static std::unique_ptr<ResultsContainer> Instance(asio::io_context &io, int tasks)
    {
        return std::unique_ptr<ResultsContainer>(new ResultsContainer_Server(io, tasks));
    }

private:
    ResultsContainer_Server(asio::io_context &io, int tasks)
        : ResultsContainer(io), _taskCount(tasks), _completedtask(0)
    {
    }

    ~ResultsContainer_Server()
    {
        DBConnManager::GetInstance().RemoveConnection(_dbConnName);
    }

    virtual bool CanStop()
    {
        return ++_completedtask >= _taskCount;
    }

    virtual void OnStopCondition()
    {
        _cond.notify_all();
    }

    virtual void OnJobCondition()
    {
        _cond.notify_one();
    }

    virtual pqxx::connection *GetConn()
    {
        std::stringstream strbld("Hydrus_Executer_thread_DBConn_");
        strbld << std::this_thread::get_id();
        _dbConnName = strbld.str();
        auto pConn = DBConnManager::GetInstance().GetConnection()->Clone();
        auto pqConn = pConn ? DBConnManager::GetInstance().MakeConnection(_dbConnName, pConn) : nullptr;
        pqxx::connection *_pqry = pqConn ? pqConn->GetConn() : nullptr;
        return _pqry;
    }

    virtual void WaitforCondition()
    {
        std::unique_lock<std::mutex> ulk(_rmux);
        _cond.wait(ulk, [&] { return !_ops.empty() || !_started; });
    }

private:
    std::string _dbConnName;
    std::condition_variable _cond;
    int _taskCount;
    int _completedtask;
};

#endif // RESULTSCONTAINER_SERVER_H
