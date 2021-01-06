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

#ifndef TCP_SERVER_HYDRUS_H
#define TCP_SERVER_HYDRUS_H

#include <limits>
#include <memory>
#include <string>
#include "asio.hpp"

namespace tinyxml2
{
    class XMLElement;
}

namespace tcp_server_inner
{
    class Responder
    {
    public:
        Responder()
        {
            _grpid = 0;
            _taskid = std::numeric_limits<int>::min();
            _respondData = "";
        }
        virtual bool Respond(std::string &answer);
    protected:
        unsigned int _grpid;
        int _taskid;
        std::string _respondData;
    };

    class ConnectionStringResponder : public Responder
    {
    public:
        ConnectionStringResponder();
    };

    class TaskIDResponder : public Responder
    {
    public:
        TaskIDResponder(tinyxml2::XMLElement *pCont);
    };

    class ParameterResponder : public Responder
    {
    public:
        ParameterResponder(tinyxml2::XMLElement *pCont);
    };

    class RunningResponder : public Responder
    {
    public:
        RunningResponder(tinyxml2::XMLElement *pCont);
    };

    class FailedResponder : public Responder
    {
    public:
        FailedResponder(tinyxml2::XMLElement *pCont);
    };

    class TimeoutResponder : public Responder
    {
    public:
        TimeoutResponder(tinyxml2::XMLElement *pCont);
    };

    class CompleteResponder : public Responder
    {
    public:
        CompleteResponder(tinyxml2::XMLElement *pCont);
    };

    class HydrusResponder : public Responder
    {
    public:
        HydrusResponder();
    };

    class TcpServer:public std::enable_shared_from_this<TcpServer>
    {
    public:
        typedef std::shared_ptr<TcpServer> ptr;
        static ptr GetServerPtr(asio::io_context &io)
        {
            ptr _new(new TcpServer(io));
            return _new;
        }
        void start()
        {
            _started=true;
            do_Read();
        }
        void stop()
        {
            if(!_started)
            {
                return;
            }
            _started=false;
            _socket.close();
        }
        bool started() const
        {
            return _started;
        }
        asio::ip::tcp::socket & sock()
        {
            return _socket;
        }
    protected:
        virtual void do_Read();
        virtual void do_Write();
        virtual void OnRead(const asio::error_code &err, size_t bytes);
        virtual void OnWrite(const asio::error_code &err, size_t bytes);
        virtual void CreateResponder(const std::string& type,tinyxml2::XMLElement* pEle);
    private:
        TcpServer(asio::io_context &io):_socket(io),_started(false)
        {
        }
        TcpServer(const TcpServer& rhs)=delete;
        TcpServer(TcpServer&& rhs)=delete;
        TcpServer& operator=(const TcpServer& rhs)=delete;
        TcpServer& operator=(TcpServer&& rhs)=delete;
        asio::ip::tcp::socket _socket;
        bool _started;
        std::string _sendbuf;
        std::string _revbuf;
        std::unique_ptr<Responder> _pResponder;
    };

} // namespace tcp_server_inner
#endif // TCP_SERVER_HYDRUS_H
