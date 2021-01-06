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

#ifndef NODETASKCONTROLLER_H
#define NODETASKCONTROLLER_H

#include <string>
#include "taskcontroller.h"

#define COMMITJOB
#ifdef ABSTRACTJOB
#include "resultcontainer.h"
#elif defined COMMITJOB
#include "asyncresultcontainer.h"
#endif
class NodeTaskController : public TaskController
{
public:
    NodeTaskController() = default;

    virtual void AddNewTask(int id)
    {
        DefaultImplete(true, id);
    }

    bool NextTaskID(int &id, unsigned int &grpid)
    {
        return DefaultImplete(false, id, grpid);
    }

    bool SetRunningState(int taskid, unsigned int grpid)
    {
        return DefaultImplete(false, taskid, grpid);
    }

    bool SetFailState(int taskid, unsigned int grpid)
    {
        return DefaultImplete(false, taskid, grpid);
    }

    bool SetTimeoutState(int taskid, unsigned int grpid)
    {
        return DefaultImplete(false, taskid, grpid);
    }

    bool SetCompleteState(int taskid, unsigned int grpid)
    {
        return DefaultImplete(false, taskid, grpid);
    }

    bool CheckLock(int taskid, unsigned int grpid)
    {
        return DefaultImplete(false, taskid, grpid);
    }

    unsigned int TaskCount() const
    {
        return 0;
    }

    bool HasNoTask();

    unsigned long RunningCount()
    {
        return 0;
    }

    unsigned long CompletedCount()
    {
        return 0;
    }

    unsigned long FailedCount()
    {
        return 0;
    }

    unsigned long TimeoutCount()
    {
        return 0;
    }

    bool PushResult(int gid, unsigned int grpid, std::string &result);

    bool RecordError(int gid, unsigned int grpid, std::string &err);

    virtual void Run();
    virtual void Run(asio::io_context &io);

protected:
    std::unique_ptr<ResultsContainer> _resultContainer;

private:
    bool DefaultImplete(bool result, ...) const
    {
        return result;
    }
};

#endif // NODETASKCONTROLLER_H
