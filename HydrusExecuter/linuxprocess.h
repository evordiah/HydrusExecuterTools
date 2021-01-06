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

#ifndef LINUXPROCESS_H
#define LINUXPROCESS_H

#include <list>
#include <string>
#include <memory>
#include <climits>
#include <atomic>

class LinuxProcess
{
    public:
        enum class state
        {
            FailedToStart,
            Crashed,
            ReadError,
            WriteError,
            Timedout,
            Success
        };

        LinuxProcess(const std::string& cmd,const std::list<std::string>& parameters);
        void WaitSeconds(unsigned int Seconds)
        {
            if(Seconds==0)
            {
                _waitSeconds=std::numeric_limits<unsigned int>::max();
            }
            else
            {
                _waitSeconds=Seconds;
            }
        }
        bool Run();
        void Terminate()
        {
            _started.store(false);
        }
        state State()
        {
            return _state;
        }
        std::string& OutPut()
        {
            return _output;
        }
        virtual ~LinuxProcess()=default;
    private:
    std::string _cmd;
    std::list<std::string> _parameterLst;
    typedef std::unique_ptr<char[]> charArrayPtr;
    std::unique_ptr<charArrayPtr[]> _argv;
    //char* *_argv;
    std::string _output;
    unsigned int _waitSeconds;
    state _state;
    std::atomic_bool  _started;
};

#endif // LINUXPROCESS_H
