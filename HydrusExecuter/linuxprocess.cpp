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
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <error.h>
#include <csignal>
#include <thread>
#include "sys/wait.h"
#include "fcntl.h"
#include "linuxprocess.h"

LinuxProcess::LinuxProcess(const std::string &cmd, const std::list<std::string> &parameters)
    : _cmd(cmd), _parameterLst(parameters), _waitSeconds(std::numeric_limits<unsigned int>::max()),
      _state(state::Success), _started(false)
{
    std::filesystem::path p = _cmd;
    _parameterLst.insert(_parameterLst.begin(), p.filename());
    //_argv=new  char *[_parameterLst.size()+1];
    _argv.reset(new charArrayPtr[_parameterLst.size()]);
    int i = 0;
    for (std::string &par : _parameterLst)
    {
        size_t nSize = par.size();
        _argv[i].reset(new char[nSize + 1]);
        par.copy(_argv[i].get(), nSize);
        _argv[i][nSize] = '\0';
        ++i;
        //_argv[i++]=(char*)par.c_str();
    }
    //_argv[i]=nullptr;
}

bool LinuxProcess::Run()
{
    bool result = true;
    int fd[2];
    int fd1[2];
    char output[1024];
    if (pipe(fd) == 0 && pipe(fd1) == 0)
    {
        close(fd1[0]);
    }
    else
    {
        _state = state::WriteError;
    }
    _started.store(true);
    auto pid = fork();
    if (pid == 0)
    {
        if (_state != state::WriteError)
        {
            close(fd[0]);
            if (fd1[1] != STDERR_FILENO)
            {
                dup2(fd1[1], STDERR_FILENO);
                close(fd1[1]);
            }
            if (fd[1] != STDOUT_FILENO)
            {
                // if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)
                // {
                //     _state = state::WriteError;
                //     result = false;
                // }
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
        }
        size_t nSize = _parameterLst.size();
        char **argv = new char *[nSize + 1];
        for (size_t i = 0; i < nSize; ++i)
        {
            argv[i] = _argv[i].get();
        }
        argv[nSize] = nullptr;
        if (execv(_cmd.c_str(), argv) == -1)
        {
            delete[] argv;
            exit(-9999);
        }
    }
    else if (pid > 0)
    {
        if (_state != state::WriteError)
        {
            close(fd[1]);
            fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL) | O_NONBLOCK);
            if (fd[0] != STDIN_FILENO)
            {
                if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
                {
                    _state = state::ReadError;
                }
                close(fd[0]);
            }
        }
        unsigned int Cnt = 0;
        int status;
        pid_t pidr;
        while (Cnt < _waitSeconds && _started.load())
        {
            if (!(pidr = waitpid(pid, &status, WNOHANG)))
            {
                if (_state != state::ReadError && _state != state::WriteError)
                {
                    std::memset(output, 0, sizeof(output));
                    if (read(STDIN_FILENO, output, sizeof(output) - 1) > 0)
                    {
                        _output.append(output);
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                Cnt++;
            }
            else
            {
                break;
            }
        }
        if (pidr == -1)
        {
            result = false;
            _state = state::Crashed;
        }
        else if (Cnt >= _waitSeconds || !_started.load())
        {
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            _state = state::Timedout;
            result = false;
        }
        else
        {
            if (WIFEXITED(status))
            {
                if (WEXITSTATUS(status) == -9999)
                {
                    _state = state::FailedToStart;
                    result = false;
                }
                else
                {
                    result = true;
                }
            }
            else
            {
                result = false;
                _state = state::Crashed;
            }
        }
        if (_state != state::ReadError && _state != state::WriteError)
        {
            do
            {
                std::memset(output, 0, sizeof(output));
                auto szt = read(STDIN_FILENO, output, sizeof(output) - 1);
                if (szt > 0)
                {
                    _output.append(output);
                }
                else if (szt == 0)
                {
                    break;
                }
            } while (true);
        }
    }
    else
    {
        _state = state::FailedToStart;
        result = false;
    }
    _started.store(false);
    return result;
}
