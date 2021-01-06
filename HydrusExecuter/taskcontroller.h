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

#ifndef TASKCONTROLLER_H
#define TASKCONTROLLER_H

#include <memory>
#include <string>
namespace asio
{
    class io_context;
}
class TaskController
{
public:
    static TaskController& GetController();

    void Release()
    {
        TaskController::pcontroler=nullptr;
    }

    virtual void AddNewTask(int id)=0;
    virtual bool NextTaskID(int &id,unsigned int &grpid)=0;
    virtual bool SetRunningState(int taskid, unsigned int grpid)=0;
    virtual bool SetFailState(int taskid, unsigned int grpid)=0;
    virtual bool SetTimeoutState(int taskid, unsigned int grpid)=0;
    virtual bool SetCompleteState(int taskid, unsigned int grpid)=0;
    virtual bool CheckLock(int taskid,unsigned int grpid)=0;
    virtual unsigned int TaskCount() const=0;
    virtual bool HasNoTask()=0;
    virtual unsigned long RunningCount()=0;
    virtual unsigned long CompletedCount()=0;
    virtual unsigned long FailedCount()=0;
    virtual unsigned long TimeoutCount()=0;
    virtual bool PushResult(int gid,unsigned int grpid, std::string& result)=0;
    virtual bool RecordError(int gid,unsigned int grpid, std::string& err)=0;
    virtual void Run()=0;
    virtual void Run(asio::io_context& io)=0;
private:
    static std::unique_ptr<TaskController> pcontroler;
};




#endif // TASKCONTROLLER_H
