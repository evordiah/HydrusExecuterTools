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

#ifndef HYDRUS_TCP_SERVER_H
#define HYDRUS_TCP_SERVER_H

#include <memory>
#include <mutex>
#include "asio.hpp"
#include "tcp_server_inner.h"

class HydrusTcpServer
{
public:
    HydrusTcpServer(asio::io_context& io) :_io(&io), _start(false)
    {
    }
    HydrusTcpServer(const HydrusTcpServer &rhs) = delete;
    HydrusTcpServer(HydrusTcpServer &&rhs) = delete;
    HydrusTcpServer &operator=(const HydrusTcpServer &rhs) = delete;
    HydrusTcpServer &operator=(HydrusTcpServer &&rhs) = delete;
    ~HydrusTcpServer() = default;
    void listen(unsigned short port);
    void stop()
    {
        _io->stop();
    }
private:
    void handle_require(tcp_server_inner::TcpServer::ptr svr, const asio::error_code &err);
    asio::io_context* _io;
    std::unique_ptr<asio::ip::tcp::acceptor> _pacceptor;
    std::mutex _mux;
    bool _start;
};
#endif // HYDRUS_TCP_SERVER_H
