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
#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include "pqdbconn.h"
#include "asio.hpp"
#include "commitjob.h"

class ResultsContainer
{
public:
    void Commit(const CommitJob &job)
    {
        bool tmpStart;
        {
            std::lock_guard<std::mutex> lk(_rmux);
            _ops.push_back(operation(_io, job,
                                     [&] {
                                         std::lock_guard<std::mutex> lk(_rmux);
                                         if (CanStop())
                                         {
                                             _started = false;
                                             OnStopCondition();
                                         }
                                     }));
            tmpStart = _started;
        }
        if (!tmpStart)
        {
            start();
        }
        OnJobCondition();
    }

protected:
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

    ResultsContainer(asio::io_context &io)
        : _io(&io), _work(new asio::io_context::work(*_io)), _started(false)
    {
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
            OnStart();
        }
        std::thread t(std::bind(&ResultsContainer::run, this));
        t.detach();
    }

    virtual void run()
    {
        pqxx::connection *_pqry = GetConn();
        while (true)
        {
            operation cur;
            {
                std::lock_guard<std::mutex> lk(_rmux);
                if (!_started)
                {
                    break;
                }
                if (!_ops.empty())
                {
                    cur = _ops[0];
                    _ops.erase(_ops.begin());
                }
            }
            if (cur._io)
            {
                cur._op(_pqry);
                cur._io->post(cur._completion);
            }
            else
            {
                WaitforCondition();
            }
        }
        _work.reset();
    }

    virtual void OnStart()
    {
    }
    virtual bool CanStop() = 0;
    virtual void OnStopCondition() = 0;
    virtual void OnJobCondition() = 0;
    virtual void WaitforCondition() = 0;
    virtual pqxx::connection *GetConn() = 0;

protected:
    asio::io_context *_io;
    work_ptr _work;
    std::mutex _rmux;
    std::vector<operation> _ops;
    bool _started;
};

#endif // RESULTSCONTAINER_H
