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
#include <cstring>
#include <Windows.h>
#include "winprocess.h"

WinProcess::WinProcess(const std::string &cmd, const std::list<std::string> &parameters)
    : _cmd(cmd), _waitSeconds((std::numeric_limits<unsigned>::max)()), _state(state::Success),
      _started(false)
{
    std::filesystem::path p = _cmd;
    // The first parameter needs to be the exe itself
    _tmpParas.append(p.filename().string());
    for (auto &e : parameters)
    {
        _tmpParas.append(" ");
        _tmpParas.append(e);
    }
}

bool WinProcess::Run()
{
    bool result = true;
    bool bClosePipe = true;
    bool bHerit=true;
    DWORD dwExitCode = 0;
    HANDLE hReadPipe = nullptr;
    HANDLE hWritePipe = nullptr;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
    {
        _state = state::WriteError;
        bClosePipe = false;
        bHerit=false;
    }
    if (_state != state::WriteError && !SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
    {
        _state = state::ReadError;
        bHerit=false;
    }
    // CreateProcessW can modify Parameters thus  allocate needed memory
    std::unique_ptr<char[]> pszParam = std::unique_ptr<char[]>(new char[_tmpParas.size() + 1]);
    if (!pszParam)
    {
        _state = state::FailedToStart;
        return false;
    }
    const char *pchrTemp = _tmpParas.c_str();
    strcpy_s(pszParam.get(), _tmpParas.size() + 1, pchrTemp);
    pszParam[_tmpParas.size()] = 0;

    /* CreateProcess API initialization */
    STARTUPINFOA siStartupInfo;
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    siStartupInfo.cb = sizeof(siStartupInfo);
    GetStartupInfoA(&siStartupInfo);
    siStartupInfo.wShowWindow = SW_HIDE;
    if (_state != state::ReadError && _state != state::WriteError)
    {
        siStartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        siStartupInfo.hStdError = hWritePipe;
        siStartupInfo.hStdOutput = hWritePipe;
    }
    else
    {
        siStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    }
    
    PROCESS_INFORMATION piProcessInfo;
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    if (CreateProcessA(_cmd.c_str(), pszParam.get(), nullptr, nullptr, bHerit, CREATE_DEFAULT_ERROR_MODE,
                       nullptr, nullptr, &siStartupInfo, &piProcessInfo) != false)
    {
        /* Watch the process. */
        _started.store(true);
        char output[1024];
        DWORD dwRead = 0;
        DWORD dwAvail = 0;
        unsigned int Cnt = 0;
        while (Cnt < _waitSeconds && _started.load())
        {
            dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, 1000);
            if (dwExitCode == WAIT_OBJECT_0)
            {
                _state = state::Success;
                result = true;
                break;
            }
            else if (WAIT_TIMEOUT == dwExitCode)
            {
                if (_state != state::WriteError && _state != state::ReadError)
                {
                    std::memset(output, 0, sizeof(output));
                    //PeekNamePipe用来预览一个管道中的数据，用来判断管道中是否为空
                    if (PeekNamedPipe(hReadPipe, nullptr, 0, &dwRead, &dwAvail, nullptr) && dwAvail > 0)
                    {
                        if (ReadFile(hReadPipe, output, sizeof(output) - 1, &dwRead, nullptr) && dwRead > 0)
                        {
                            _output.append(output);
                        }
                    }
                }
                Cnt++;
            }
            else if (WAIT_FAILED == dwExitCode)
            {
                _state = state::Crashed;
                result = false;
                break;
            }
        }
        if (Cnt >= _waitSeconds || !_started.load())
        {
            while (true)
            {
                if (TerminateProcess(piProcessInfo.hProcess, 0))
                {
                    WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
                    break;
                }
            }
            _state = state::Timedout;
            result = false;
        }
        if (_state != state::WriteError && _state != state::ReadError)
        {
            do
            {
                if (!PeekNamedPipe(hReadPipe, nullptr, NULL, &dwRead, &dwAvail, nullptr) || dwAvail <= 0)
                {
                    break;
                }
                std::memset(output, 0, sizeof(output));
                if (ReadFile(hReadPipe, output, sizeof(output) - 1, &dwRead, nullptr))
                {
                    if (dwRead == 0)
                    {
                        break;
                    }
                    _output.append(output);
                }
            } while (true);
        }
    }
    else
    {
        /* CreateProcess failed */
        _state = state::FailedToStart;
        result = false;
    }
    CloseHandle(piProcessInfo.hProcess);
    CloseHandle(piProcessInfo.hThread);
    if (bClosePipe)
    {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
    }
    _started.store(false);
    return result;
}
