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

#ifndef RESULTSCONTAINER_NODE_H
#define RESULTSCONTAINER_NODE_H

#include "applicationparametermanager.h"
#include "asyncresultcontainer.h"

class ResultsContainer_Node : ResultsContainer
{
public:
    ResultsContainer_Node(const ResultsContainer_Node &rhs) = delete;
    ResultsContainer_Node(ResultsContainer_Node &&rhs) = delete;
    ResultsContainer_Node &operator=(const ResultsContainer_Node &rhs) = delete;
    ResultsContainer_Node &operator=(ResultsContainer_Node &&rhs) = delete;

    static std::unique_ptr<ResultsContainer> Instance(asio::io_context &io)
    {
        return std::unique_ptr<ResultsContainer>(new ResultsContainer_Node(io));
    }

private:
    ResultsContainer_Node(asio::io_context &io)
        : ResultsContainer(io), _shouldWait(true)
    {
        std::shared_ptr<asio::steady_timer> pt(new asio::steady_timer(*_io, std::chrono::milliseconds(1000)));
        pt->async_wait(std::bind(&ResultsContainer_Node::WaitStarting, this, std::placeholders::_1, pt));
    }

    void WaitStarting(const asio::error_code &, std::shared_ptr<asio::steady_timer> pt)
    {
        const int watingtime = 60;

        static int i = 1;
        {
            std::lock_guard<std::mutex> lk(_rmux);
            if (!_shouldWait)
            {
                return;
            }
            if (!_started && i++ < watingtime)
            {
                pt->expires_at(pt->expiry() + std::chrono::milliseconds(1000));
                pt->async_wait(std::bind(&ResultsContainer_Node::WaitStarting, this, std::placeholders::_1, pt));
            }
            else if (!_started && i >= watingtime)
            {
                _work.reset();
            }
        }
    }

    virtual void OnStart()
    {
        _shouldWait = false;
    }
    
    virtual bool CanStop()
    {
        return _ops.empty() &&
               0 == ApplicationParameterManager::RunningThreadCount();
    }

    virtual void OnStopCondition()
    {
    }

    virtual void OnJobCondition()
    {
    }

    virtual pqxx::connection *GetConn()
    {
        return nullptr;
    }

    virtual void WaitforCondition()
    {
        while (true)
        {
            {
                std::lock_guard<std::mutex> lk(_rmux);
                if (!_ops.empty() || !_started)
                {
                    break;
                }
                if (CanStop())
                {
                    _started = false;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

private:
    bool _shouldWait;
};
#endif // RESULTSCONTAINER_NODE_H
