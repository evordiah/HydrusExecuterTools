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

#ifndef SERVERTASKCONTROLLER_H
#define SERVERTASKCONTROLLER_H


#include "taskcontroller.h"
#include "taskcontainer.h"
#define COMMITJOB
#ifdef ABSTRACTJOB
#include "resultcontainer.h"
#elif defined COMMITJOB
#include "asyncresultcontainer.h"
#endif

class ServerTaskController:public TaskController
{
public:
    ServerTaskController()=default;

    virtual void AddNewTask(int id)
    {
        _taskContainer.Add(id);
    }

    virtual bool NextTaskID(int &id,unsigned int& grpid);

    virtual bool SetRunningState(int taskid, unsigned int grpid)
    {
        return _taskContainer.State(taskid,grpid,TaskContainer::running);
    }

    virtual bool SetFailState(int taskid, unsigned int grpid)
    {
        return _taskContainer.State(taskid,grpid,TaskContainer::failed);
    }

    virtual bool SetTimeoutState(int taskid, unsigned int grpid)
    {
        return _taskContainer.State(taskid,grpid,TaskContainer::timeout);
    }

    virtual bool SetCompleteState(int taskid, unsigned int grpid)
    {
        return _taskContainer.State(taskid,grpid,TaskContainer::completed);
    }

    virtual bool CheckLock(int taskid,unsigned int grpid)
    {
        return _taskContainer.State(taskid,grpid)==TaskContainer::locked;
    }

    virtual unsigned int TaskCount() const
    {
        return _taskContainer.TaskCount();
    }

    virtual bool HasNoTask()
    {
        return _taskContainer.HasNoTask() ;
    }

    virtual unsigned long RunningCount()
    {
        return _taskContainer.RunningCount();
    }

    virtual unsigned long CompletedCount()
    {
        return _taskContainer.CompletedCount();
    }

    virtual unsigned long FailedCount()
    {
        return _taskContainer.FailedCount();
    }

    virtual unsigned long TimeoutCount()
    {
        return _taskContainer.TimeoutCount();
    }

    virtual bool PushResult(int gid,unsigned int grpid, std::string& result);

    virtual bool RecordError(int gid, unsigned int grpid, std::string &err);

    virtual void Run();
    virtual void Run(asio::io_context& io);
protected:
    TaskContainer _taskContainer;
    std::unique_ptr<ResultsContainer> _resultContainer;
};

#endif // SERVERTASKCONTROLLER_H
