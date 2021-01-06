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

#ifndef APPLICATIONPARAMETERMANAGER_H
#define APPLICATIONPARAMETERMANAGER_H

#include <atomic>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <iostream>
#include <string>

class ApplicationParameterManager
{
    friend class CommandLineParser;

public:
    static ApplicationParameterManager &Instance()
    {
        if (!_pApp)
        {
            _pApp.reset(new ApplicationParameterManager());
        }
        return *_pApp;
    }
    ApplicationParameterManager(const ApplicationParameterManager &rhs) = delete;
    ApplicationParameterManager(ApplicationParameterManager &&rhs) = delete;
    ApplicationParameterManager &operator=(const ApplicationParameterManager &rhs) = delete;
    ApplicationParameterManager &operator=(ApplicationParameterManager &&rhs) = delete;

    static unsigned int RunningThreadCount()
    {
        return _gRunningThreadCount.load();
    }

    static void RegisterThreadStarting()
    {
        _gRunningThreadCount.fetch_add(1);
    }

    static void RegisterThreadFinished()
    {
        _gRunningThreadCount.fetch_sub(1);
    }

    ~ApplicationParameterManager();

    void Release()
    {
        ApplicationParameterManager::_pApp = nullptr;
    }

    unsigned int NewGroupID()
    {
        return _GroupID.fetch_add(1);
    }

    unsigned int WaitSeconds() const
    {
        return _gWaitSec;
    }

    const std::string &LogFile() const
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

    unsigned int ThreadCount();

    const std::string &HydursExecutingPath() const
    {
        return _AppExecutingTempPath;
    }

    bool RunOnServer() const
    {
        return _bRunOnServer;
    }

    const std::string &HostIP() const
    {
        return _strip;
    }

    unsigned short HostPort() const
    {
        return _port;
    }

    void RecordErrorInFile(int gid, const std::string &err);
    void RecordErrorInDatabase();

    const std::string GetProgramPath(const std::string &program);

private:
    ApplicationParameterManager();

    void WaitSeconds(unsigned int sec)
    {
        if (sec > 0)
        {
            _gWaitSec = sec;
        }
    }

    void HydursExecutingPath(const std::string &path)
    {
        _AppExecutingTempPath = path;
    }

    void LogFile(const std::string &logfile)
    {
        _glogFile = logfile;
    }

    void RunOnServer(bool bVal)
    {
        _bRunOnServer = bVal;
    }

    void ThreadCount(unsigned int i)
    {
        _gThreadCnt = i;
    }

    void HostIP(const std::string &ip)
    {
        _strip = ip;
    }

    void HostPort(unsigned short port)
    {
        _port = port;
    }

    void LogErrorInFile();

private:
    static std::unique_ptr<ApplicationParameterManager> _pApp;
    static std::atomic<unsigned int> _gRunningThreadCount;

private:
    std::string _strip;
    unsigned short _port;
    bool _bRunOnServer;
    bool _gErrInDb;
    bool _gErrInFile;
    unsigned int _gThreadCnt;
    unsigned int _gWaitSec;
    std::atomic<unsigned int> _GroupID;
    std::string _AppExecutingTempPath;
    std::string _glogFile;
    std::mutex _gErrMutex;
    std::unique_ptr<std::map<int, std::string>> _pErrorsInFile;
    std::map<std::string, std::string> _programPath;
};

#endif // APPLICATIONPARAMETERMANAGER_H
