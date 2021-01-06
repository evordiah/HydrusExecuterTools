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

#ifndef SERVERPROCESSINDICATOR_H
#define SERVERPROCESSINDICATOR_H
#include "asio.hpp"

class ServerProcessIndicator
{
public:
    ServerProcessIndicator(asio::io_context &io) : _io(&io), _started(false), _milliseconds(1000)
    {
    }
    void start(int milliseconds);
    void stop();

private:
    void ShowResult();
    void ProcessIdle(const asio::error_code & /*e*/, asio::steady_timer *t);
    asio::io_context *_io;
    bool _started;
    int _milliseconds;
};

#endif // SERVERPROCESSINDICATOR_H
