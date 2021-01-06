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

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <limits>
#include <memory>
#include <string>
#include <functional>
#include "asio.hpp"

namespace tcp_client_inner
{
    enum class enumRequire : int
    {
        requireConnectionString = 0,
        requireHydrus = 1,
        requireTaskID = 2,
        requireParameter = 3,
        notifyRunning = 4,
        notifyFail = 5,
        notifyTimeout = 6,
        notifyComplete = 7
    };

    enum class enumTcpSocketState : int
    {
        Failed = 0,
        Succeed = 1,
        FatalError = 2
    };

    class Requirer
    {
    public:
        Requirer()
        {
            _grpid = 0;
            _taskid = std::numeric_limits<int>::min();
            _bCommitToDatabase = false;
            _senddata = "";
            _revdata = "";
            _userdata = "";
        }

        void GroupID(unsigned int grpid)
        {
            _grpid = grpid;
        }

        unsigned int GroupID()
        {
            return _grpid;
        }

        void TaskID(int taskid)
        {
            _taskid = taskid;
        }

        int TaskID()
        {
            return _taskid;
        }

        void Data(std::string &data)
        {
            if (data.empty())
            {
                return;
            }
            _senddata = std::move(data);
            _bCommitToDatabase = true;
        }

        std::string Data()
        {
            if (_revdata.empty())
            {
                return "";
            }
            return std::move(_revdata);
        }

        void UserData(std::string &data)
        {
            if (data.empty())
            {
                return;
            }
            _userdata = std::move(data);
        }

        std::string UserData()
        {
            if (_userdata.empty())
            {
                return "";
            }
            return std::move(_userdata);
        }

        virtual bool Require(std::string &require) = 0;
        virtual bool Receive(std::string &answer) = 0;

    protected:
        //send data
        unsigned int _grpid;
        int _taskid;
        bool _bCommitToDatabase;
        std::string _senddata;
        std::string _revdata;
        std::string _userdata;
    };

    class BaseRequirer : public Requirer
    {
    public:
        BaseRequirer() = default;
        bool Require(std::string &require);
        bool Receive(std::string &answer);

    protected:
        virtual bool PrePareData() = 0;
        virtual bool ParseAnswerData() = 0;
    };

    class ConnectionStringRequirer : public BaseRequirer
    {
    public:
        ConnectionStringRequirer() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class HydrusRequier : public BaseRequirer
    {
    public:
        HydrusRequier() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class TaskIDRequirer : public BaseRequirer
    {
    public:
        TaskIDRequirer() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class ParameterRequirer : public BaseRequirer
    {
    public:
        ParameterRequirer() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class RunningNotify : public BaseRequirer
    {
    public:
        RunningNotify() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class FailedNotify : public BaseRequirer
    {
    public:
        FailedNotify() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class TimeoutNotify : public BaseRequirer
    {
    public:
        TimeoutNotify() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class CompleteNotify : public BaseRequirer
    {
    public:
        CompleteNotify() = default;

    protected:
        bool PrePareData();
        bool ParseAnswerData();
    };

    class Tcp_Client : public std::enable_shared_from_this<Tcp_Client>
    {
    public:
        typedef std::shared_ptr<Tcp_Client> ptr;
        static ptr NewConnection(asio::io_context &io, enumRequire enumr)
        {
            std::shared_ptr<Tcp_Client> _newConn(new Tcp_Client(io, enumr));
            return _newConn;
        }

        void start(asio::ip::tcp::endpoint ep)
        {
            _started = true;
            _socket.async_connect(ep, std::bind(&Tcp_Client::onConnect, shared_from_this(), std::placeholders::_1));
        }

        void GroupID(unsigned int grpid)
        {
            _pRequirer->GroupID(grpid);
        }

        unsigned int GroupID() const
        {
            return _pRequirer->GroupID();
        }

        void TaskID(int taskid)
        {
            _pRequirer->TaskID(taskid);
        }

        int TaskID() const
        {
            return _pRequirer->TaskID();
        }

        void Data(std::string &data)
        {
            _pRequirer->Data(data);
        }

        std::string Data() const
        {
            return std::move(_pRequirer->Data());
        }

        void UserData(std::string &data)
        {
            _pRequirer->UserData(data);
        }

        std::string UserData() const
        {
            return std::move(_pRequirer->UserData());
        }

        void State(enumTcpSocketState bval)
        {
            _State = bval;
        }

        void State(bool bval)
        {
            _State = bval ? enumTcpSocketState::Succeed : enumTcpSocketState::Failed;
        }

        enumTcpSocketState State() const
        {
            return _State;
        }

        bool started()
        {
            return _started;
        }

        void stop()
        {
            if (!_started)
            {
                return;
            }
            _started = false;
            _socket.close();
        }

    protected:
        virtual void onConnect(const asio::error_code &err);
        virtual void OnRead(const asio::error_code &err, size_t bytes);
        virtual void OnWrite(const asio::error_code &err, size_t bytes);

    private:
        Tcp_Client(asio::io_context &io, enumRequire enumr);
        Tcp_Client(const Tcp_Client &r) = delete;
        Tcp_Client(Tcp_Client &&r) = delete;
        Tcp_Client &operator=(const Tcp_Client &r) = delete;
        Tcp_Client &operator=(Tcp_Client &&r) = delete;

    private:
        asio::ip::tcp::socket _socket;
        bool _started;
        enumTcpSocketState _State;
        std::string _sendbuf;
        std::string _revbuf;
        std::unique_ptr<Requirer> _pRequirer;
    };
} // namespace tcp_client_inner
#endif // TCPSOCKET_H
