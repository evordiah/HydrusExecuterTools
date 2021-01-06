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
#include <chrono>
#include <functional>
#include "clientprocessindicator.h"
#include "Stringhelper.h"

void ClientProcessIndicator::start(int milliseconds)
{
    _started = true;
    if (milliseconds > 0)
    {
        _milliseconds = milliseconds;
    }
    _startTm = std::time(nullptr);
    asio::steady_timer t(*_io, std::chrono::milliseconds(_milliseconds));
    t.async_wait(std::bind(&ClientProcessIndicator::ProcessIdle, this, std::placeholders::_1, &t));
    _io->run();
    ShowResult();
}

void ClientProcessIndicator::stop()
{
    if (_started)
    {
        _io->stop();
    }
}

void ClientProcessIndicator::ShowResult()
{
    std::cout << std::endl;
    std::cout << "\n==============Hydrus has successfully finished.==============\n\n";
}

void ClientProcessIndicator::ProcessIdle(const asio::error_code & /*e*/, asio::steady_timer *t)
{
    static bool bfirst = true;
    char buf[80];
    time_t tm = std::time(nullptr);
    double tmdiff = std::difftime(tm, _startTm);
    struct tm *local = localtime(&tm);
    std::strftime(buf, 80, "%h/%d %H:%M:%S", local);
    std::string message;
    if (bfirst)
    {
        message = "Operation is exectuing, please wait...";
        std::cout << message << std::endl;
        bfirst = false;
    }
    message = "time: %1, has run %2 seconds";
    Stringhelper sh(message);
    sh.arg(std::string(buf));
    sh.arg(tmdiff);
    std::stringstream strbld;
    strbld << sh.str();
    /* static int i = 0;
    switch (i++)
    {
    case 0:
        break;
    case 1:
        strbld << ".";
        break;
    case 2:
        strbld << "..";
        break;
    case 3:
        strbld << "...";
        break;
    case 4:
        strbld << "....";
        break;
    case 5:
        strbld << ".....";
        break;
    case 6:
        strbld << "......";
        i = 0;
        break;
    }*/
    message = std::string(message.size() + 1, ' ');
    std::cout << "\r" << message << "\r" << strbld.str() << std::flush;
    if (_started)
    {
        t->expires_at(t->expiry() + std::chrono::milliseconds(_milliseconds));
        t->async_wait(std::bind(&ClientProcessIndicator::ProcessIdle, this, std::placeholders::_1, t));
    }
}
