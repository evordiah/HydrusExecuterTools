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

#define COMMITJOB
//#define ABSTRACTJOB

#include <sstream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <memory>
#include <atomic>
#include "applicationparametermanager.h"
#include "pqdbconn.h"
#ifdef ABSTRACTJOB
#include "abstractjob.h"
#elif defined COMMITJOB
#include "commitjob.h"
#endif

class ResultsContainer
{
public:
    ResultsContainer(size_t MaxTask = 0)
        : _data(new data(MaxTask))
    {
        _threadCnt.store(0);
    }

    void Start(size_t thread_count)
    {
        auto l = [&] {
            std::string dbConn;
            pqxx::connection *pqry = nullptr;
            if (_data->_maxTaskCnt)
            {
                std::stringstream strbld("Hydrus_Executer_thread_DBConn_");
                strbld << std::this_thread::get_id();
                dbConn = strbld.str();
                auto pConn = DBConnManager::GetInstance().GetConnection()->Clone();
                auto pqConn = DBConnManager::GetInstance().MakeConnection(dbConn, pConn);
                if (!pqConn || !(pqry = pqConn->GetConn()))
                {
                    DBConnManager::GetInstance().RemoveConnection(dbConn);
                    return;
                }
            }
            _threadCnt.fetch_add(1);
            std::unique_lock<std::mutex> lk(_data->_mtx);
            while (true)
            {
#ifdef COMMITJOB
                if (!_data->_Committasks.empty())
                {
                    auto current = _data->_Committasks.front();
                    _data->_Committasks.pop();
                    lk.unlock();
                    current(pqry);
#elif defined ABSTRACTJOB
                if (!_data->_Abstracttasks.empty())
                {
                    auto current = std::move(_data->_Abstracttasks.front());
                    _data->_Abstracttasks.pop();
                    lk.unlock();
                    if (pqry)
                    {
                        current->operator()(*pqry);
                    }
                    else
                    {
                        current->operator()();
                    }
#endif
                    lk.lock();
                    if (_data->_maxTaskCnt && ++_data->_CompletedTaskCnt >= _data->_maxTaskCnt)
                    {
                        if (!_data->_is_shutdown)
                        {
                            _data->_is_shutdown = true;
                            _data->_cond.notify_all();
                        }
                        _threadCnt.fetch_sub(1);
                        _cond.notify_one();
                        break;
                    }
                    else if (!_data->_maxTaskCnt && !ApplicationParameterManager::RunningThreadCount())
                    {
                        if (!_data->_is_shutdown)
                        {
                            _data->_is_shutdown = true;
                            _data->_cond.notify_all();
                        }
                    }
                }
                else if (_data->_is_shutdown)
                {
                    _threadCnt.fetch_sub(1);
                    _cond.notify_one();
                    break;
                }
                else
                {
                    _data->_cond.wait(lk);
                }
            }

            if (pqry)
            {
                pqry = nullptr;
                DBConnManager::GetInstance().RemoveConnection(dbConn);
            }
        };
        for (size_t i = 0; i < thread_count; ++i)
        {
            std::thread(l).detach();
        }
    }

    void WaitForFinished()
    {
        if (!_data)
        {
            return;
        }
        std::unique_lock<std::mutex> ul(_mtx);
        while (true)
        {
            if (_threadCnt.load() == 0)
            {
                break;
            }
            _cond.wait(ul);
        }
    }

    void Stop()
    {
        if (_data)
        {
            {
                std::lock_guard<std::mutex> lk(_data->_mtx);
                _data->_is_shutdown = true;
            }
            _data->_cond.notify_all();
        }
    }

    ~ResultsContainer()
    {
        Stop();
    }

#ifdef ABSTRACTJOB
    void Commit(std::unique_ptr<AbstractJob> &&task)
    {
        {
            std::lock_guard<std::mutex> lk(_data->_mtx);
            _data->_Abstracttasks.emplace(std::move(task));
        }
        _data->_cond.notify_one();
    }
#elif defined COMMITJOB
    void Commit(const CommitJob &job)
    {
        {
            std::lock_guard<std::mutex> lk(_data->_mtx);
            _data->_Committasks.emplace(job);
        }
        _data->_cond.notify_one();
    }
#endif

private:
    struct data
    {
        data(size_t maxTaskcnt)
            : _maxTaskCnt(maxTaskcnt), _CompletedTaskCnt(0), _is_shutdown(false)
        {
        }
        std::mutex _mtx;
        std::condition_variable _cond;
        size_t _maxTaskCnt;
        size_t _CompletedTaskCnt;
#ifdef ABSTRACTJOB
        std::queue<std::unique_ptr<AbstractJob>> _Abstracttasks;
#elif defined COMMITJOB
        std::queue<CommitJob> _Committasks;
#endif
        bool _is_shutdown;
    };
    std::shared_ptr<data> _data;
    std::atomic<size_t> _threadCnt;
    std::mutex _mtx;
    std::condition_variable _cond;
};

#endif // RESULTSCONTAINER_H
