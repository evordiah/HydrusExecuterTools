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

#include <filesystem>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <memory>
#include "applicationparametermanager.h"
#include "taskcontroller.h"

#if defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
#include <error.h>
#endif

std::unique_ptr<ApplicationParameterManager> ApplicationParameterManager::_pApp(nullptr);
std::atomic<unsigned int> ApplicationParameterManager::_gRunningThreadCount{0};

ApplicationParameterManager::ApplicationParameterManager():_GroupID(1)
{
    _gWaitSec=0;
    _gThreadCnt=1;
    _bRunOnServer=false;
    _gErrInDb=false;
    _gErrInFile=false;
    _AppExecutingTempPath="";
    _glogFile = "";
    _strip="";
    _port=0;
}


#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
const std::string ApplicationParameterManager::GetProgramPath(const std::string &program)
{
    return "";
}
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
const std::string ApplicationParameterManager::GetProgramPath(const std::string &program)
{
    auto it=_programPath.find(program);
    if(it!=_programPath.end())
    {
        return it->second;
    }
    std::string result;
    std::string cmd("which ");
    cmd+=program;
    FILE *fpipe=::popen(cmd.c_str(),"r");
    if(nullptr!=fpipe)
    {
        char output[1024];
        std::memset(output,0,sizeof (output));
        while(std::fgets(output,sizeof(output),fpipe))
        {
            std::string tmp(output);
            if(tmp.back()=='\n')
            {
                tmp.pop_back();
            }
            result.append(tmp);
        }
        ::pclose(fpipe);
    }
    _programPath.insert(std::make_pair(program,result));
    return result;
}
#else
#   error unknown os
#endif

ApplicationParameterManager::~ApplicationParameterManager()
{
    if(_pErrorsInFile)
    {
        LogingErrorInFile();
    }
}

unsigned int ApplicationParameterManager::ThreadCount()
{
    static bool first=true;
    if(first && _bRunOnServer)
    {
        unsigned int cnt=TaskController::GetController().TaskCount();
        if(_gThreadCnt>cnt)
        {
            _gThreadCnt=cnt;
        }
        first=false;
    }
    return _gThreadCnt;
}

void ApplicationParameterManager::RecordErrorInFile(int gid, const std::string& err)
{
    {
        std::lock_guard<std::mutex> lg(_gErrMutex);
        if(!_pErrorsInFile)
        {
            _pErrorsInFile = std::make_unique<std::map<int,std::string>>();
        }
        _pErrorsInFile->insert(std::make_pair(gid,err));
        _gErrInFile=true;
    }
}

void ApplicationParameterManager::RecordErrorInDatabase()
{
    {
        std::lock_guard<std::mutex> lg(_gErrMutex);
        _gErrInDb=true;
    }
}

void ApplicationParameterManager::LogErrorInFile()
{
    std::ofstream out( this->_glogFile,std::ios_base::app);
    if(!out )
    {
        _pErrorsInFile.reset(nullptr);
        return;
    }
    for(auto it=_pErrorsInFile->begin();it!=_pErrorsInFile->end();++it)
    {
        out<<it->first<<" :\t"<<it->second<<std::endl;
    }
    out.close();
    _pErrorsInFile.reset(nullptr);
}
