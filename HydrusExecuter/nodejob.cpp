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

#include <thread>
#include <regex>
#include <fstream>
#include <filesystem>
#include "nodejob.h"
#include "applicationparametermanager.h"
#include "tcp_client_inner.h"

NodeJob::NodeJob(int gid, unsigned int grpid, std::string &val, bool berr)
    : _gid(gid), _grpid(grpid), _bSaveInFile(false)
{
    if(!_perr)
    {
        _perr=std::make_shared<std::string>();
    }
    if(!_presult)
    {
        _presult=std::make_shared<std::string>();
    }
    if (berr)
    {
        *_perr = std::move(val);
        *_presult = "";
    }
    else
    {
        *_perr = "";
        *_presult = std::move(val);
        std::filesystem::path p = ApplicationParameterManager::Instance().HydursExecutingPath();
        p /= "\\d+\\.RES";
        //std::regex r(p.native());
		std::regex r(p.string());
        if (std::regex_match(*_presult, r) && std::filesystem::exists(*_presult))
        {
            _bSaveInFile = true;
        }
    }
}

void NodeJob::CommitError()
{
    int iCount = 0;
    while (true)
    {
        switch (tcp_client_inner::enumTcpSocketState(SendErr(_gid, _grpid)))
        {
        case tcp_client_inner::enumTcpSocketState::FatalError:
            if (++iCount > 9)
            {
                return;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            break;
        default:
            return;
        }
    }
}

void NodeJob::CommitResult()
{
    int iCount = 0;
    while (true)
    {
        switch (tcp_client_inner::enumTcpSocketState(SendResult(_gid, _grpid)))
        {
        case tcp_client_inner::enumTcpSocketState::FatalError:
            if (++iCount > 9)
            {
                return;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            break;
        default:
            return;
        }
    }
}

void NodeJob::operator()()
{
    if (!_bSaveInFile)
    {
        if (!_presult->empty())
        {
            CommitResult();
        }
        else
        {
            CommitError();
        }
    }
    else
    {
        ImportFromFile();
    }
}

int NodeJob::SendResult(int gid, unsigned int grpid)
{
    asio::io_context io;
    auto tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::notifyComplete);
    const std::string &ip = ApplicationParameterManager::Instance().HostIP();
    unsigned short port = ApplicationParameterManager::Instance().HostPort();
    tcp->GroupID(grpid);
    tcp->TaskID(gid);
    tcp->Data(*_presult);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
    tcp->start(ep);
    io.run();
    return int(tcp->State());
}

int NodeJob::SendErr(int gid, unsigned int grpid)
{
    asio::io_context io;
    tcp_client_inner::Tcp_Client::ptr tcp;
    if (_perr->find("timeout") != std::string::npos)
    {
        tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::notifyTimeout);
    }
    else
    {
        tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::notifyFail);
    }
    const std::string &ip = ApplicationParameterManager::Instance().HostIP();
    unsigned short port = ApplicationParameterManager::Instance().HostPort();
    tcp->GroupID(grpid);
    tcp->TaskID(gid);
    tcp->Data(*_perr);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
    tcp->start(ep);
    io.run();
    return int(tcp->State());
}

void NodeJob::ImportFromFile()
{
    std::ifstream in(*_presult);
    if (!in)
    {
        //if failed change self to a error object
        std::error_code ec;
        std::filesystem::remove(*_presult, ec);
        _presult->clear();
        *_perr = "import failed!";
        CommitError();
        return;
    }
    std::string val;
    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty())
        {
            val.append(line);
        }
    }
    in.close();
    std::error_code ec;
    std::filesystem::remove(*_presult, ec);
    _presult->clear();
    *_presult = std::move(val);
    CommitResult();
}