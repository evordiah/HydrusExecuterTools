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

#include "taskcontainer.h"
#include "applicationparametermanager.h"

TaskContainer::Seconds TaskContainer::GetRunningTimout()
{
    double w;
    {
        std::lock_guard<std::mutex> lg1(_TaskRegisterMutex);
        w = _ComputingMeanTime;
    }
    if (w < 1e-5)
    {
        w = 15 * 60;
    }
    return Seconds(w*4);
}

bool TaskContainer::StillLockOrRunning(unsigned int grpid)
{
    for (auto it = _LockedTask.begin(); it != _LockedTask.end(); ++it)
    {
        if (it->second.first == grpid)
        {
            return true;
        }
    }
    for (auto it = _RunningTask.begin(); it != _RunningTask.end(); ++it)
    {
        if (it->second.first == grpid)
        {
            return true;
        }
    }
    return false;
}

bool TaskContainer::GetFreeTaskId(int &taskid, unsigned int grpid)
{
    std::chrono::steady_clock::time_point ntm = std::chrono::steady_clock::now();
    int waiting = ApplicationParameterManager::Instance().WaitSeconds();
    if (!waiting)
    {
        waiting = 30;
    }
    Seconds locktimeout(waiting * 2);
    Seconds runtimeout = GetRunningTimout();
    {
        std::lock_guard<std::mutex> lg1(_TaskRegisterMutex);
        if (!_gids.empty())
        {
            auto it = _gids.begin();
            taskid = *it;
            _gids.erase(it);
            _LockedTask[taskid] = std::make_pair(grpid, std::chrono::steady_clock::now());
            return true;
        }
        for (auto it = _LockedTask.begin(); it != _LockedTask.end(); ++it)
        {
            auto stm = it->second.second;
            if ((ntm - stm) > locktimeout)
            {
                taskid = it->first;
                it->second = std::make_pair(grpid, std::chrono::steady_clock::now());
                return true;
            }
        }
        for (auto it = _RunningTask.begin(); it != _RunningTask.end(); ++it)
        {
            auto stm = it->second.second;
            Seconds _tmpRuntimeout=runtimeout;
            if ((ntm - stm) > runtimeout)
            {
                int tmp = 1;
                auto itrob = _robedCount.find(it->first);
                if (itrob == _robedCount.end())
                {
                    _robedCount.insert(std::make_pair(it->first, tmp));
                }
                else
                {
                    tmp = ++itrob->second;
                }
                _tmpRuntimeout = runtimeout * tmp;
            }
            if ((ntm - stm) > _tmpRuntimeout)
            {
                taskid = it->first;
                _RunningTask.erase(it);
                _LockedTask[taskid] = std::make_pair(grpid, std::chrono::steady_clock::now());
                return true;
            }
        }
        if ((_gids.empty() && _RunningTask.empty() && _LockedTask.empty()) ||
            (ApplicationParameterManager::Instance().ThreadCount() >= (_LockedTask.size() + _RunningTask.size())))
        {
                        //if the _LockedTask or _RunningTask has not yet been dealt
            //then still waiting for 5 seconds
            if(StillLockOrRunning(grpid))
            {
                taskid=5;
            }
            else    //zero taskid and false return means the client thread can quit
            {
                taskid = 0;
            } 
        }
        else
        {
            //non-zero taskid and false return means the client thread should
            //wait for taskid seconds and
            //require again to get a new taskid
            taskid = waiting;
        }
        return false;
    }
}

bool TaskContainer::State(int taskid, unsigned int grpid, TaskContainer::EnumTaskState state)
{
    bool result = false;
    std::chrono::steady_clock::time_point nt = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lg1(_TaskRegisterMutex);
        switch (state)
        {
        //the running state is come from locked state
        case running:
        {
            auto iterator2 = _LockedTask.find(taskid);
            if (iterator2 != _LockedTask.end() && iterator2->second.first == grpid)
            {
                _LockedTask.erase(iterator2);
                _RunningTask[taskid] = std::make_pair(grpid, std::chrono::steady_clock::now());
                result = true;
            }
        }
        break;
        // if running task failed, the state is set failed, somtimes the task is
        // completed successfully, but not imported into the database, the state
        // is considered failed, the state is changed to failed from complete.
        // And if a locked task can not be got parameters, it can not enter running
        // state, so it will be failed, it is the third state that changed from locked
        // into failed directly without running state
        case failed:
        {
            auto iterator3 = _RunningTask.find(taskid);
            if (iterator3 != _RunningTask.end() && iterator3->second.first == grpid)
            {
                _RunningTask.erase(iterator3);
                _FailedTask[taskid] = grpid;
                _robedCount.erase(taskid);
                result = true;
            }
            if (!result)
            {
                auto iterator = _CompletedTask.find(taskid);
                if (iterator != _CompletedTask.end() && iterator->second == grpid)
                {
                    _CompletedTask.erase(iterator);
                    _FailedTask[taskid] = grpid;
                    result = true;
                }
            }
            if (!result)
            {
                auto iterator1 = _LockedTask.find(taskid);
                if (iterator1 != _LockedTask.end() && iterator1->second.first == grpid)
                {
                    _LockedTask.erase(iterator1);
                    _FailedTask[taskid] = grpid;
                    _robedCount.erase(taskid);
                    result = true;
                }
            }
        }
        break;
        // only a running task can be changed to timeout
        case timeout:
        {
            auto iterator4 = _RunningTask.find(taskid);
            if (iterator4 != _RunningTask.end() && iterator4->second.first == grpid)
            {
                _RunningTask.erase(iterator4);
                _TimeoutTask[taskid] = grpid;
                _robedCount.erase(taskid);
                result = true;
            }
        }
        break;
        case completed:
        {
            auto iterator5 = _RunningTask.find(taskid);
            if (iterator5 != _RunningTask.end() && iterator5->second.first == grpid)
            {
                double t = std::chrono::duration_cast<Seconds>(nt - iterator5->second.second).count() ;
                _RunningTask.erase(iterator5);
                _CompletedTask[taskid] = grpid;
                _robedCount.erase(taskid);
                _ComputingMeanTime += (t - _ComputingMeanTime) / _CompletedTask.size();
                result = true;
            }
        }
        break;
        default:
            break;
        }
    }
    return result;
}

TaskContainer::EnumTaskState TaskContainer::State(int taskid, unsigned int grpid)
{
    std::lock_guard<std::mutex> lg1(_TaskRegisterMutex);
    auto iteratorlock = _LockedTask.find(taskid);
    if (iteratorlock != _LockedTask.end())
    {
        if (iteratorlock->second.first == grpid)
        {
            return locked;
        }
        return unKnown;
    }
    auto iteratorrun = _RunningTask.find(taskid);
    if (iteratorrun != _RunningTask.end())
    {
        if (iteratorrun->second.first == grpid)
        {
            return running;
        }
        return unKnown;
    }
    auto iteratorfail = _FailedTask.find(taskid);
    if (iteratorfail != _FailedTask.end())
    {
        if (iteratorfail->second == grpid)
        {
            return failed;
        }
        return unKnown;
    }
    auto iteratortimeout = _TimeoutTask.find(taskid);
    if (iteratortimeout != _TimeoutTask.end())
    {
        if (iteratortimeout->second == grpid)
        {
            return timeout;
        }
        return unKnown;
    }
    auto iteratorcomplete = _CompletedTask.find(taskid);
    if (iteratorcomplete != _CompletedTask.end())
    {
        if (iteratorcomplete->second == grpid)
        {
            return completed;
        }
        return unKnown;
    }
    return free;
}

bool TaskContainer::HasNoTask()
{
    std::lock_guard<std::mutex> lg1(_TaskRegisterMutex);
    return _gids.empty() && _RunningTask.empty() && _LockedTask.empty();
}
