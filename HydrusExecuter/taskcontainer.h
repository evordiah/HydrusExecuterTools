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

#ifndef TASKCONTAINER_H
#define TASKCONTAINER_H

#include <set>
#include <mutex>
#include <map>
#include <chrono>

class TaskContainer
{
public:
    enum EnumTaskState
    {
        free,
        locked,
        running,
        failed,
        completed,
        timeout,
        unKnown
    };

    TaskContainer()
    {
        _gCounter = 0;
        _ComputingMeanTime = 0;
    }

    void Add(int id)
    {
        if (_gids.insert(id).second)
        {
            _gCounter++;
        }
    }

    unsigned long RunningCount() const
    {
        return _RunningTask.size();
    }

    unsigned long CompletedCount() const
    {
        return _CompletedTask.size();
    }

    unsigned long FailedCount() const
    {
        return _FailedTask.size();
    }

    unsigned long TimeoutCount() const
    {
        return _TimeoutTask.size();
    }

    unsigned int TaskCount() const
    {
        return _gCounter;
    }
    bool GetFreeTaskId(int &taskid, unsigned int grpid);
    bool State(int taskid, unsigned int grpid, EnumTaskState state);
    EnumTaskState State(int taskid, unsigned int grpid);
    bool HasNoTask();

protected:
    typedef std::chrono::duration<double> Seconds;
    Seconds GetRunningTimout();
 	bool StillLockOrRunning(unsigned int grpid);
private:
    //task id list
    //std::mutex _gidsMutex;
    std::set<int> _gids;

    std::mutex _TaskRegisterMutex;
    std::map<int, std::pair<unsigned int, std::chrono::steady_clock::time_point>> _LockedTask;
    std::map<int, std::pair<unsigned int, std::chrono::steady_clock::time_point>> _RunningTask;
    std::map<int, unsigned int> _FailedTask;
    std::map<int, unsigned int> _TimeoutTask;
    std::map<int, unsigned int> _CompletedTask;
    std::map<int, unsigned short> _robedCount;
    unsigned int _gCounter;
    double _ComputingMeanTime;
};

#endif // TASKCONTAINER_H
