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
#include <queue>
#include <condition_variable>
#include <mutex>
#include <QSqlQuery>
#include <memory>
#include <map>

class QSqlDatabase;

class TaskController
{
public:
    static TaskController& GetController()
    {
        if(!pcontroler)
        {
            pcontroler.reset(new TaskController());
        }
        return *pcontroler;
    }

    void Release()
    {
        TaskController::pcontroler=nullptr;
    }

    void WaitHydrusForSeconds(unsigned int sec)
    {
        if(sec>0)
        {
            _gWaitforHydrus=sec;
        }
    }
    
    unsigned int WaitHydrusForSeconds() const
    {
        return _gWaitforHydrus;
    }
    
    void AddNewTask(int id)
    {
        _gids.push(id);
        _gCounter++;
    }

    bool RemoveUnProcessTask(int &id);

    void PushResult(const int gid,std::unique_ptr<std::string> &result);

    unsigned int TaskCount() const
    {
        return _gCounter;
    }

    bool ShouldRun() const
    {
        return _gCounter>0?true:false;
    }

    void ThreadCount(unsigned int i)
    {
        if(i>1)
        {
            _gThreadCnt=i;
        }
    }

    unsigned int ThreadCount() const
    {
        if(_gThreadCnt<_gCounter)
        {
            return _gThreadCnt;
        }
        else
        {
            return _gCounter;
        }
    }

    void LogFile(const std::string& logfile)
    {
        _glogFile=logfile;
    }

    std::string LogFile() const
    {
        return _glogFile;
    }

    bool ExistErrors() const
    {
        return _gErrInDb || _gErrInFile;
    }

    bool LogingErrorInDB() const
    {
        return _gErrInDb;
    }

    bool LogingErrorInFile() const
    {
        return _gErrInFile;
    }

    void RunEvenly(QSqlDatabase& db,const std::string& exepath );

    void Run(QSqlDatabase& db,const std::string& exepath);

    void RecordError(int gid,const std::string & err);

    std::string GetProgramPath(const std::string& program);
protected:
    void DealResult(QSqlQuery* qry);
    void InsertintoDB(const int gid, std::unique_ptr<std::string>&, QSqlQuery* qry);
    void GetTaskList(std::vector<int> &vecgid, unsigned int threadindex);
    void LogErrorInFile();
    bool LogErrorInDB(QSqlQuery* qry);
private:
    TaskController();

    std::mutex _gbufResMutex;
    std::condition_variable _gbufCondVar;
    std::queue<std::pair<int,std::unique_ptr<std::string>>> _gbufresultsqueue;
    std::queue<std::pair<int,std::unique_ptr<std::string>>> _gbuftoinserdb;

    std::mutex _gidsMutex;
    std::queue<int> _gids;

    std::mutex _errMutex;
    std::map<int,std::string> _gErrs;

    std::string _glogFile;

    std::map<std::string,std::string> _programPath;

    unsigned int _gCounter;
    unsigned int _gThreadCnt;
    unsigned int _gWaitforHydrus;
    bool _gErrInDb;
    bool _gErrInFile;

private:
    static std::unique_ptr<TaskController> pcontroler;
};

#endif // TASKCONTROLLER_H
