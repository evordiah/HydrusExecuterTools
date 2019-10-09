
/****************************************************************************** 
 * 
 * 
 *  Copyright (c) 2019, Wenzhao Feng.
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

#include <sstream>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <fstream>
#include <future>
#include <thread>
#include <atomic>
#include <QUuid>
#include "HydrusExcuter.h"
#include "HydrusResultCompresser.h"
#include "exporthydrusinputfile.h"


extern bool _gErrInDb;
std::queue<std::string> HydrusExcuter::bufresultsqueue;
std::mutex HydrusExcuter::bufResMutex;
std::condition_variable HydrusExcuter::bufCondVar;
std::queue<int> HydrusExcuter::gids;
std::mutex HydrusExcuter::gidsMutex;

HydrusExcuter::HydrusExcuter(const std::string &exepath)
{
     _exepath.setPath(exepath.c_str());
     _valid=false;
}

HydrusExcuter::~HydrusExcuter()
{
    _pqry->finish();
    _db.close();
}

void HydrusExcuter::DataBase(const QSqlDatabase &db)
{
    QUuid id=QUuid::createUuid();
    _db=QSqlDatabase::cloneDatabase(db,id.toString());
    _valid=_db.open();
    if(!_valid)
    {
        std::cout<<"Can't connect database!"<<std::endl;
    }
}

bool HydrusExcuter::InitialProcess()
{
    auto tid=std::this_thread::get_id();
    std::stringstream  strbld;
    strbld<<"hydrus"<<tid;
    _pqry.reset(new QSqlQuery(_db));
    std::string subdir=_exepath.absoluteFilePath(strbld.str().c_str()).toStdString();
    _currentprocesspath.setPath(subdir.c_str());
    if(!_currentprocesspath.exists())
    {
        if(!_currentprocesspath.mkpath(subdir.c_str()))
        {
            return false;
        }
    }
    return true;
}

bool HydrusExcuter::PreapareParameterFile(int gid)
{
    ExportHydrusInputFile exp(_pqry);
    exp.Gid(gid);
    return exp.Execute(QDir::toNativeSeparators(_currentprocesspath.absolutePath()).toStdString());
}

void HydrusExcuter::LogError(int gid,const std::string& error)
{
    std::stringstream strbld;
    strbld<<"insert into errlog(gid, errordes) values("<<gid<<",'"<<error<<"');";
    _gErrInDb = true;
    _pqry->exec(strbld.str().c_str());
}

bool HydrusExcuter::GetId(int &gid)
{
    gid=-9999;
    return false;
}

void HydrusExcuter::ExecuteHydrus(int gid)
{
    auto pid=fork();
    if(pid==0)
    {
        std::string exefile= QDir::toNativeSeparators(_exepath.absoluteFilePath("hydrus.exe")).toStdString();
        std::string pcpath=QDir::toNativeSeparators(_currentprocesspath.absolutePath()).toStdString();
        if(execl("/bin/wine","wine",exefile.c_str(),pcpath.c_str(),nullptr)==-1)
        {
            LogError(gid,"hydrus execute failed");
            _Exit(EXIT_FAILURE);
        }
    }
    else if(pid>0)
    {
        if(waitpid(pid,nullptr,0)==-1)
            LogError(gid,"hydrus execute failed");
    }
    else
    {
        LogError(gid,"can not fork");
    }
}

void HydrusExcuter::CompressResults(int gid)
{
    std::stringstream strbld;
    strbld<<gid<<".bin";
    std::string savepath=QDir::toNativeSeparators(_exepath.absoluteFilePath(strbld.str().c_str())).toStdString();
    //if(HydrusResultCompresser::Compress(_currentprocesspath.string(),savepath.string(),false))
    //{
    //    {
    //        std::lock_guard<std::mutex> lg(bufResMutex);
    //        bufresultsqueue.push(savepath.string());
    //    }
    //    bufCondVar.notify_one();
    //}
    //wheather success or failure, the gid has been dealed with, the bufresultsqueue should record
    //the addressed gid, and this make main procedure exit normal
    std::string filename=savepath;
    if(!HydrusResultCompresser::Compress(QDir::toNativeSeparators(_currentprocesspath.absolutePath()).toStdString(),filename,false))
    {
        filename="error";
    }
    {
        std::lock_guard<std::mutex> lg(bufResMutex);
        bufresultsqueue.push(filename);
    }
    bufCondVar.notify_one();
}

void HydrusExcuter::ClearTempFile()
{
    auto List = _currentprocesspath.entryInfoList();
    while(List.size())
    {
        auto qfinfo =List.back();
        List.pop_back();
        _currentprocesspath.remove(qfinfo.absoluteFilePath());
    }
}

void HydrusExcuter::operator()()
{
   Execute();
}

void HydrusExcuter::Execute()
{
    if(!_valid || !InitialProcess())
    {
        return;
    }
    int gid;
    while(GetId(gid))
    {
        if(PreapareParameterFile(gid))
        {
            ExecuteHydrus(gid);
            CompressResults(gid);
            ClearTempFile();
        }
        else
        {
            {
                std::lock_guard<std::mutex> lg(bufResMutex);
                bufresultsqueue.push("error");
            }
            bufCondVar.notify_one();
        }
    }
    _currentprocesspath.removeRecursively();
}
