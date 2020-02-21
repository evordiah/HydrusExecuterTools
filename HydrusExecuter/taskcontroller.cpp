
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <future>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QCoreApplication>
#include "HydrusExcuter.h"
#include "hydrusexcuterevenly.h"
#include "hydrusexcuterpre.h"
#include "taskcontroller.h"

std::unique_ptr<TaskController> TaskController::pcontroler(nullptr);

void TaskController::LogErrorInFile()
{
    if(!_gErrs.empty())
    {
        std::ofstream out(_glogFile,std::ios_base::app);
        if(!out )
        {
            return;
        }
        for(auto it=_gErrs.begin();it!=_gErrs.end();++it)
        {
            out<<it->first<<" :\t"<<it->second<<std::endl;
        }
        out.close();
        _gErrInFile = true;
    }
}

bool TaskController::LogErrorInDB(QSqlQuery *qry)
{
    if(!_gErrs.empty())
    {
        std::stringstream strbld;
        strbld<<"insert into errlog(gid, errordes) values ";
        for(auto it=_gErrs.begin();it!=_gErrs.end();++it)
        {
            strbld<<"("<<it->first<<",'"<<it->second<<"'),";
        }
        std::string sqlstm=strbld.str();
        sqlstm.back()=';';
        _gErrInDb=qry->exec(sqlstm.c_str());
    }
    return _gErrInDb;
}

TaskController::TaskController()
{
    _gErrInDb=false;
    _gErrInFile=false;
    _gWaitforHydrus=0;
    _gThreadCnt=1;
    _gCounter=0;
#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
    QDir currentpath= QCoreApplication::applicationDirPath();
    _glogFile = QDir::toNativeSeparators(currentpath.absoluteFilePath("hydrusexecutingerrlog.log")).toStdString();
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
    _glogFile="/tmp/hydrusexecutingerrlog.log";
#else
#   error unknown os
#endif
}

void TaskController::DealResult(QSqlQuery *qry)
{
    unsigned int tCounter=_gCounter;
    while(tCounter)
    {
        //std::this_thread::sleep_for(std::chrono::seconds(20));
        {
            std::unique_lock<std::mutex> ul(_gbufResMutex);
            _gbufCondVar.wait(ul,[this] {return !_gbufresultsqueue.empty();});
            while(!_gbufresultsqueue.empty())
            {
                _gbuftoinserdb.push(std::move(_gbufresultsqueue.front()));
                _gbufresultsqueue.pop();
            }
        }
        while(!_gbuftoinserdb.empty())
        {
            auto v=std::move(_gbuftoinserdb.front());
            _gbuftoinserdb.pop();
            if(v.second)
            {
                InsertintoDB(v.first,v.second,qry);
            }
            tCounter--;
        }
    }

    if(!_gErrs.empty() && !LogErrorInDB(qry))
    {
        LogErrorInFile();
    }
}

void TaskController::InsertintoDB(const int gid, std::unique_ptr<std::string>& pSqlStmt, QSqlQuery *qry)
{
    try
    {
        if(!qry->exec(pSqlStmt->c_str()))
        {
            RecordError(gid,"import failed!");
        }
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

void TaskController::GetTaskList(std::vector<int>& vecgid,unsigned int threadindex)
{
    vecgid.clear();
    unsigned int gthread=ThreadCount();
    unsigned int cnt=_gCounter/gthread;
    unsigned int res=_gCounter-gthread*cnt;
    for(unsigned int i=0;i<cnt;++i)
    {
        vecgid.push_back(_gids.front());
        _gids.pop();
    }
    if(threadindex<res)
    {
        vecgid.push_back(_gids.front());
        _gids.pop();
    }
}

bool TaskController::RemoveUnProcessTask(int &id)
{
    bool result;
    {
        std::lock_guard<std::mutex> lg(_gidsMutex);
        if((result=!_gids.empty()))
        {
            id=_gids.front();
            _gids.pop();
        }

    }
    return result;
}

void TaskController::PushResult(const int gid,std::unique_ptr<std::string>& result)
{
    {
        std::lock_guard<std::mutex> lg(_gbufResMutex);
        _gbufresultsqueue.push(std::make_pair(gid,std::move(result)));
    }
    _gbufCondVar.notify_one();
}

void TaskController::RunEvenly(QSqlDatabase &db, const std::string &exepath)
{
    std::vector<std::shared_ptr<HydrusExcuter>> _executers;
    std::vector<std::future<void>> _futures;
    std::shared_ptr<HydrusExcuter> pHydrusExe;
    std::vector<int> vecgid;
    for(unsigned int i=0;i<ThreadCount();++i)
    {
        GetTaskList(vecgid,i);
        pHydrusExe.reset(new HydrusExcuterEvenly(exepath,vecgid));
        pHydrusExe->DataBase(db);
        _executers.push_back(pHydrusExe);
        _futures.push_back(std::async(std::launch::async, &HydrusExcuter::Execute,_executers[i]));
    }
    std::unique_ptr<QSqlQuery> qry(new QSqlQuery(db));
    DealResult(qry.get());
    qry->finish();
    _executers.clear();
    _futures.clear();
    QDir qexepath(exepath.c_str());
    qexepath.removeRecursively();
}

void TaskController::Run(QSqlDatabase &db,const std::string& exepath)
{
    std::vector<std::shared_ptr<HydrusExcuter>> _executers;
    std::vector<std::future<void>> _futures;
    std::shared_ptr<HydrusExcuter> pHydrusExe;
    for(unsigned int i=0;i<ThreadCount();++i)
    {
        pHydrusExe.reset(new HydrusExcuterPre(exepath));
        pHydrusExe->DataBase(db);
        _executers.push_back(pHydrusExe);
        _futures.push_back(std::async(std::launch::async, &HydrusExcuter::Execute,_executers[i]));
    }
    std::unique_ptr<QSqlQuery> qry(new QSqlQuery(db));
    DealResult(qry.get());
    qry->finish();
    _executers.clear();
    _futures.clear();
    QDir qexepath(exepath.c_str());
    qexepath.removeRecursively();
}

void TaskController::RecordError(int gid, const std::string &err)
{
    std::lock_guard<std::mutex> lg(_errMutex);
    _gErrs[gid]=err;
}

#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
std::string TaskController::GetProgramPath(const std::string &program)
{
    return "";
}
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
std::string TaskController::GetProgramPath(const std::string &program)
{
    if(_programPath.find(program)==_programPath.end())
    {
        QProcess p;
        p.start("which",QStringList()<<program.c_str());
        if(p.waitForFinished(-1) || (p.error()==QProcess::UnknownError
                                     && p.exitStatus()==QProcess::NormalExit))
        {
            char output[1024];
            p.setReadChannel(QProcess::StandardOutput);
            p.readLine(output,1024);
            std::string str=QString(output).trimmed().toStdString();
            if(str.empty())
            {
                _programPath[program]= "";
            }
            else
            {
                _programPath[program]=str;
            }
        }
        else
        {
            _programPath[program]= "";
        }
    }
    return _programPath[program];
}
#else
#   error unknown os
#endif
