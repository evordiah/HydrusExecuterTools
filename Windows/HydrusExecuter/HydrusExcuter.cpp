
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
#include<Windows.h>
#include <fstream>
#include <future>
#include <thread>
#include <atomic>
#include <QUuid>  
#include <QDir>
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
	QUuid id = QUuid::createUuid();
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
	if (!_currentprocesspath.exists())
	{
		if (!_currentprocesspath.mkpath(subdir.c_str()))
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
	DWORD dwExitCode = 0;
	std::string exefile = QDir::toNativeSeparators(_exepath.absoluteFilePath("hydrus.exe")).toStdString();
	// The first parameter needs to be the exe itself 
	std::string sTempStr = "hydrus.exe ";
	sTempStr.append(QDir::toNativeSeparators(_currentprocesspath.absolutePath()).toStdString());
	// CreateProcessW can modify Parameters thus  allocate needed memory 
	std::unique_ptr<char[]> pszParam = std::unique_ptr<char[]>(new char[sTempStr.size() + 1]);
	if (!pszParam)
	{
		LogError(gid, "can not execute hydurs.exe,have no enough memory!");
		return;
	}
	const char* pchrTemp = sTempStr.c_str();
	strcpy_s(pszParam.get(), sTempStr.size() + 1, pchrTemp);
	pszParam[sTempStr.size()] = 0;

	/* CreateProcess API initialization */
	STARTUPINFOA siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	if (CreateProcessA(exefile.c_str(), pszParam.get(), nullptr, nullptr, false, CREATE_DEFAULT_ERROR_MODE,
		nullptr, nullptr, &siStartupInfo, &piProcessInfo) != false)
	{
		/* Watch the process. */
		dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
		if (dwExitCode)
		{
			LogError(gid, "hydrus execute failed!");
		}
	}
	else
	{
		/* CreateProcess failed */
		LogError(gid, "can not execute hydrus, can not create thread in windows!");
	}

	CloseHandle(piProcessInfo.hProcess);
	CloseHandle(piProcessInfo.hThread);
}

void HydrusExcuter::CompressResults(int gid)
{
    std::stringstream strbld;
    strbld<<gid<<".bin";
	std::string savepath =QDir::toNativeSeparators(_exepath.absoluteFilePath(strbld.str().c_str())).toStdString();
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
	while (List.size())
	{
		auto qfinfo = List.back();
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
