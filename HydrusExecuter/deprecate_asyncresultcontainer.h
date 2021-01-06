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

#ifndef RESULTSCONTAINER_H
#define RESULTSCONTAINER_H

#include <functional>
#include <sstream>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>
#include <memory>
#include <atomic>
#include "applicationparametermanager.h"
#include "pqdbconn.h"
#include "asio.hpp"
#include "commitjob.h"

class ResultsContainer
{
public:
    ResultsContainer(const ResultsContainer &rhs) = delete;
    ResultsContainer(ResultsContainer &&rhs) = delete;
    ResultsContainer &operator=(const ResultsContainer &rhs) = delete;
    ResultsContainer &operator=(ResultsContainer &&rhs) = delete;

    static std::unique_ptr<ResultsContainer> Instance(asio::io_context &io, int tasks = 0)
    {
        return std::unique_ptr<ResultsContainer>(new ResultsContainer(io, tasks));
    }

    void Commit(const CommitJob &job)
    {
        bool tmpStart;
        {
            std::lock_guard<std::mutex> lk(_rmux);
            _ops.push_back(operation(_io, job,
                                     [&] {
                                         std::lock_guard<std::mutex> lk(_rmux);
                                         if ((_taskCount && ++_completedtask >= _taskCount) ||
                                             (0 == _taskCount &&
                                              _ops.empty() &&
                                              0 == ApplicationParameterManager::RunningThreadCount()))
                                         {
                                             _started = false;
                                             _cond.notify_all();
                                             //wait all the threads stop
                                             //std::this_thread::sleep_for(std::chrono::seconds(3));
                                             _work.reset();
                                         }
                                     }));
            tmpStart = _started;
        }
        if (!tmpStart)
        {
            start();
        }
        _cond.notify_one();
    }
    // void Stop()
    // {
    //     std::lock_guard<std::mutex> lk(_rmux);
    //     if (!_started)
    //     {
    //         return;
    //     }
    //     _ops.clear();
    //     _started = false;
    //     _cond.notify_all();
    //     //wait all the threads stop
    //     //std::this_thread::sleep_for(std::chrono::seconds(3));
    //     _work.reset();
    // }

private:
    typedef std::function<void()> completion_func;
    typedef std::function<void(pqxx::connection *)> op_func;
    typedef std::shared_ptr<asio::io_context::work> work_ptr;
    struct operation
    {
        operation(asio::io_context *io, op_func op, completion_func completion)
            : _io(io), _op(op), _completion(completion)
        {
        }
        operation() : _io(nullptr)
        {
        }
        asio::io_context *_io;
        op_func _op;
        completion_func _completion;
    };

    ResultsContainer(asio::io_context &io, int tasks = 0)
        : _io(&io), _work(new asio::io_context::work(*_io)),
          _taskCount(tasks), _completedtask(0), _started(false)
    {
        if (!tasks) //run on node
        {
            std::shared_ptr<asio::steady_timer> pt(new asio::steady_timer(*_io, std::chrono::milliseconds(1000)));
            pt->async_wait(std::bind(&ResultsContainer::WaitStarting,this,std::placeholders::_1, pt));
        }
    }

    void WaitStarting(const asio::error_code &, std::shared_ptr<asio::steady_timer> pt)
    {
        static int i = 1;
        {
            std::lock_guard<std::mutex> lk(_rmux);
            if (!_started && i++<60)
            {
                pt->expires_at(pt->expiry() + std::chrono::milliseconds(1000));
                pt->async_wait(std::bind(&ResultsContainer::WaitStarting,this,std::placeholders::_1,pt));
            }
            else if(!_started && i>=60)
            {
                _work.reset();
            }
        }
    }


    void start()
    {
        {
            std::lock_guard<std::mutex> lk(_rmux);
            if (_started)
            {
                return;
            }
            _started = true;
        }
        std::thread t(std::bind(&ResultsContainer::run, this));
        t.detach();
    }

    void run()
    {
        if (_taskCount)
        {
            runserver();
        }
        else
        {
            runnode();
        }
    }

    void runserver()
    {
        std::stringstream strbld("Hydrus_Executer_thread_DBConn_");
        strbld << std::this_thread::get_id();
        std::string _dbConnName = strbld.str();
        strbld.str("");
        auto pConn = DBConnManager::GetInstance().GetConnection()->Clone();
        auto pqConn = pConn ? DBConnManager::GetInstance().MakeConnection(_dbConnName, pConn) : nullptr;
        pqxx::connection *_pqry = pqConn ? pqConn->GetConn() : nullptr;
        if (_pqry)
        {
            while (true)
            {
                {
                    std::lock_guard<std::mutex> lk(_rmux);
                    if (!_started)
                    {
                        break;
                    }
                }
                operation cur;
                {
                    std::unique_lock<std::mutex> ulk(_rmux);
                    if (!_ops.empty())
                    {
                        cur = _ops[0];
                        _ops.erase(_ops.begin());
                    }
                    else
                    {
                        _cond.wait(ulk, [&] { return !_ops.empty() || !_started; });
                        continue;
                    }
                }
                cur._op(_pqry);
                cur._io->post(cur._completion);
            }
        }
        DBConnManager::GetInstance().RemoveConnection(_dbConnName);
    }

    void runnode()
    {
        while (true)
        {
            {
                std::lock_guard<std::mutex> lk(_rmux);
                if (!_started)
                {
                    break;
                }
            }
            operation cur;
            {
                std::unique_lock<std::mutex> ulk(_rmux);
                if (!_ops.empty())
                {
                    cur = _ops[0];
                    _ops.erase(_ops.begin());
                }
                else
                {
                    _cond.wait(ulk, [&] { return !_ops.empty() || !_started; });
                    continue;
                }
            }
            cur._op(nullptr);
            cur._io->post(cur._completion);
        }
    }

    asio::io_context *_io;
    work_ptr _work;
    std::mutex _rmux;
    std::vector<operation> _ops;
    std::condition_variable _cond;
    int _taskCount;
    int _completedtask;
    bool _started;
};

#endif // RESULTSCONTAINER_H
