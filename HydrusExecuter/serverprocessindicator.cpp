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
#include <functional>
#include <ctime>
#include "applicationparametermanager.h"
#include "taskcontroller.h"
#include "serverprocessindicator.h"

void ServerProcessIndicator::start(int milliseconds)
{
    _started = true;
    if (milliseconds > 0)
    {
        _milliseconds = milliseconds;
    }
    asio::steady_timer t(*_io, std::chrono::milliseconds(_milliseconds));
    t.async_wait(std::bind(&ServerProcessIndicator::ProcessIdle, this, std::placeholders::_1, &t));
    _io->run();
    ShowResult();
}

void ServerProcessIndicator::stop()
{
    //     if (_started)
    //     {
    //         _started = false;
    //     }
    if (_started)
    {
        _io->stop();
    }
}

void ServerProcessIndicator::ShowResult()
{
    if(TaskController::GetController().TaskCount()==0)
    {
        return;
    }
    std::cout << std::endl;
    if (ApplicationParameterManager::Instance().ExistErrors())
    {
        std::cout << "\n==============Hydrus has finished "
                  << TaskController::GetController().TaskCount() << " cases with errors.==============\n";
        if (ApplicationParameterManager::Instance().LogingErrorInDB())
        {
            std::cout << "!!!!!!!!!! There are some errors during runing. See table errlog for more details !!!!!!!!!!\n\n";
        }
        else if (ApplicationParameterManager::Instance().LogingErrorInFile())
        {
            std::cout << "!!!!!!!!!! There are some errors during runing. See file"
                      << ApplicationParameterManager::Instance().LogFile() << " for more details !!!!!!!!!!\n\n";
        }
    }
    else
    {
        std::cout << "\n==============Hydrus has successfully finished. "
                  << TaskController::GetController().TaskCount() << " cases have been done.==============\n\n";
    }
}

void ServerProcessIndicator::ProcessIdle(const asio::error_code & /*e*/, asio::steady_timer *t)
{
    static int i = 0;
    if (!i)
    {
        std::string message = "Operation is exectuing, please wait";
        std::cout << message << std::endl;
        std::string message1 = " runs(C = completed | F = failed | T = timed out |R = running)";
        std::cout << message1 << std::endl;
        std::string splitline = "---------------------------------------------------------------";
        std::cout << splitline << std::endl;
        i = 1;
    }
    int c = TaskController::GetController().CompletedCount();
    int r = TaskController::GetController().RunningCount();
    int f = TaskController::GetController().FailedCount();
    int tcnt = TaskController::GetController().TimeoutCount();
    std::stringstream strbld;
    char buf[80];
    time_t tm = std::time(nullptr);
    struct tm *local = localtime(&tm);
    std::strftime(buf, 80, "%h/%d %H:%M:%S", local);
    strbld << std::string(buf);
    strbld << "  runs(C=" << c << "\t| F=" << f << "\t| T=" << tcnt << "\t| R=" << r << ")";
    std::string message=strbld.str();
    i=message.size()>i?message.size()+1:i;
    std::cout <<"\r"<<std::string(i,' ')<< "\r" << message << std::flush;
    if (_started)
    {
        t->expires_at(t->expiry() + std::chrono::milliseconds(_milliseconds));
        t->async_wait(std::bind(&ServerProcessIndicator::ProcessIdle, this, std::placeholders::_1, t));
    }
}
