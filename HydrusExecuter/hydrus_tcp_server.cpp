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

#include <memory>
#include <thread>
#include <functional>
#include "hydrus_tcp_server.h"

void HydrusTcpServer::handle_require(tcp_server_inner::TcpServer::ptr svr, const asio::error_code &err)
{
    svr->start();
    tcp_server_inner::TcpServer::ptr new_svr = tcp_server_inner::TcpServer::GetServerPtr(*_io);
    _pacceptor->async_accept(new_svr->sock(), std::bind(&HydrusTcpServer::handle_require, this, new_svr, std::placeholders::_1));
}

void HydrusTcpServer::listen(unsigned short port)
{
    if (_start)
    {
        return;
    }
    _start = true;
    asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port);
    _pacceptor = std::make_unique<asio::ip::tcp::acceptor>(*_io, ep);
    tcp_server_inner::TcpServer::ptr svr = tcp_server_inner::TcpServer::GetServerPtr(*_io);
    _pacceptor->async_accept(svr->sock(), std::bind(&HydrusTcpServer::handle_require, this, svr, std::placeholders::_1));
    _io->run();
}
