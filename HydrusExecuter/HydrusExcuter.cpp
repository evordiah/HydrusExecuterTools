
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

#include <sstream>
#include <iostream>
//#include <sys/types.h>
//#include <unistd.h>
//#include <error.h>
#include <fstream>
#include <future>
#include <thread>
#include <atomic>
#include <QUuid>
#include <QProcess>
#include "HydrusExcuter.h"
#include "taskcontroller.h"

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
        std::cerr<<"Can't connect database!"<<std::endl;
    }
}

bool HydrusExcuter::InitialThread()
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
    std::string tmppath=QDir::toNativeSeparators(_currentprocesspath.absolutePath()).toStdString();
    _HydrusFilesManager.reset(new HydrusParameterFilesManager(gid,tmppath,*_pqry));
    return _HydrusFilesManager->ExportInputFiles();
}

bool HydrusExcuter::GetId(int &gid)
{
    gid=-9999;
    return false;
}

bool HydrusExcuter::ExecuteHydrus(int gid)
{
    bool result=true;
    QString exefile= QDir::toNativeSeparators(_exepath.absoluteFilePath("hydrus.exe"));
    QString pcpath=QDir::toNativeSeparators(_currentprocesspath.absolutePath());
    QProcess p;
    std::string winepath=TaskController::GetController().GetProgramPath("wine");
    if(winepath.empty())
    {
        p.start(exefile,QStringList()<<pcpath);
    }
    else
    {
        p.start(winepath.c_str(),QStringList()<<exefile<<pcpath);
    }
    int msecs=static_cast<int>(TaskController::GetController().WaitHydrusForSeconds());
    msecs*=1000;
    if(!msecs)
    {
        msecs=-1;
    }
    if(!p.waitForFinished(msecs))
    {
        switch (p.error())
        {
        case QProcess::FailedToStart:
            TaskController::GetController().RecordError(gid,"hydrus Failed to start");
            result=false;
            break;
        case QProcess::Crashed:
        case QProcess::WriteError:
        case QProcess::ReadError:
            TaskController::GetController().RecordError(gid,"hydrus Failed to execute");
            result=false;
            break;
        case QProcess::Timedout:
            p.kill();
            p.waitForFinished(-1);
            TaskController::GetController().RecordError(gid,"hydrus time out");
            result=false;
            break;
        case QProcess::UnknownError:
            break;
        }
    }
    return result;
}

void HydrusExcuter::ClearTempFile()
{
    auto List = _currentprocesspath.entryInfoList(QDir::QDir::Files);
    while(!List.empty())
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
    if(!_valid || !InitialThread())
    {
        return;
    }
    int gid;
    while(GetId(gid))
    {
        bool result=false;
        if(PreapareParameterFile(gid))
        {
            if(ExecuteHydrus(gid))
            {
                auto r=_HydrusFilesManager->GetImportResultFilesSQlStatement();
                if(r)
                {
                    if(_HydrusFilesManager->HasErr())
                    {
                        TaskController::GetController().RecordError(gid,_HydrusFilesManager->ErrMessage());
                    }
                    else
                    {
                        TaskController::GetController().PushResult(gid,r);
                        result=true;
                    }
                }
            }
            ClearTempFile();
            _HydrusFilesManager=nullptr;
        }
        if(!result)
        {
            std::unique_ptr<std::string> nullp;
            TaskController::GetController().PushResult(gid,nullp);
        }
    }
    _currentprocesspath.removeRecursively();
}
