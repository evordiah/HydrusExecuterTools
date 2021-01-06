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
#include <regex>
#include <filesystem>
#include <system_error>
#include <fstream>
#include <sstream>
#include <thread>
#include "commitjob.h"
#include "resultcontainer.h"
#include "applicationparametermanager.h"
#include "nodejob.h"
#include "applicationparametermanager.h"
#include "tcp_client_inner.h"
#include "pqdbconn.h"

CommitJob::CommitJob(int gid, unsigned int grpid,
                     std::string &val, bool bErr)
    : _gid(gid), _grpid(grpid), _bSaveInFile(false)
{
    if (!_perr)
    {
        _perr = std::make_shared<std::string>();
    }
    if (!_presult)
    {
        _presult = std::make_shared<std::string>();
    }
    if (bErr)
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

CommitJob::CommitJob(int gid, unsigned int grpid)
    : _gid(gid), _grpid(grpid), _bSaveInFile(false)
{
}

void CommitJob::CommitError()
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

void CommitJob::CommitError(pqxx::connection &qry)
{
    std::string sqlst = Stringhelper("insert into errlog(gid, errordes) values (%1,'%2');").arg(_gid).arg(*_perr).str();
    try
    {
        pqxx::work w(qry);
        w.exec(sqlst);
        w.commit();
        ApplicationParameterManager::Instance().RecordErrorInDatabase();
    }
    catch (std::exception &e)
    {
        std::string reason = e.what();
        _perr->append(reason);
        ApplicationParameterManager::Instance().RecordErrorInFile(_gid, *_perr);
        std::cout << reason << std::endl;
    }
}

void CommitJob::CommitResult()
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

void CommitJob::CommitResult(pqxx::connection &qry)
{
    try
    {
        pqxx::work w(qry);
        w.exec(*_presult);
        w.commit();
    }
    catch (std::exception &e)
    {
        //if failed change self to a error object
        std::string reason = e.what();
        std::cout << reason << std::endl;
        _presult->clear();
        *_perr = "import failed!";
        _perr->append(reason);
        CommitError(qry);
    }
}

void CommitJob::operator()(pqxx::connection *qry)
{
    if (qry)
    {
        operator()(*qry);
    }
    else
    {
        operator()();
    }
}

void CommitJob::operator()(pqxx::connection &qry)
{
    if (!_bSaveInFile)
    {
        if (!_presult->empty())
        {
            CommitResult(qry);
        }
        else
        {
            CommitError(qry);
        }
    }
    else
    {
        ImportFromFile(qry);
    }
}

void CommitJob::operator()()
{
    if (_presult && !_presult->empty())
    {
        CommitResult();
    }
    else if (_perr && !_perr->empty())
    {
        CommitError();
    }
}

int CommitJob::SendResult(int gid, unsigned int grpid)
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

int CommitJob::SendErr(int gid, unsigned int grpid)
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

void CommitJob::SplitSqlState(const std::string &state, std::list<std::string> &sqls) const
{
    std::regex sep("[ \t\n]*;[ \t\n]*");
    std::sregex_token_iterator p(state.cbegin(), state.cend(), sep, -1);
    std::sregex_token_iterator e;
    for (; p != e; ++p)
    {
        std::string token = p->str();
        if (token.back() != ';')
        {
            token.push_back(';');
        }
        if (std::regex_search(token, std::regex("INSERT\\s+INTO", std::regex::icase)))
        {
            if (token.size() > 1024 * 1024)
            {
                SplitInsertSqlState(token, sqls);
            }
            else
            {
                sqls.push_back(token);
            }
        }
        else
        {
            if (sqls.empty())
            {
                sqls.push_back(token);
            }
            else if (sqls.back().size() + token.size() < 1024 * 1024)
            {
                sqls.back().append(token);
            }
            else
            {
                sqls.push_back(token);
            }
        }
    }
}

void CommitJob::SplitInsertSqlState(const std::string &state, std::list<std::string> &sqls) const
{
    if (!std::regex_search(state, std::regex("\\)\\s*,")))
    {
        sqls.push_back(state);
        return;
    }
    std::smatch m;
    std::string head;
    if (std::regex_search(state, m, std::regex("VALUES", std::regex::icase)))
    {
        head = state.substr(0, m.position());
    }
    int nLength = state.size();
    const int size = 1024 * 1024;
    int spos = 0, epos = 0, range = size;
    std::string tmp;
    while (spos + size < nLength)
    {
        tmp = state.substr(spos, range);
        if (!std::regex_search(tmp, m, std::regex("\\)\\s*,")))
        {
            range += size;
        }
        else
        {
            epos = m.position() + m.length(0);
            tmp = tmp.substr(0, epos - 1);
            tmp.append(";");
            if (spos == 0)
            {
                sqls.push_back(tmp);
            }
            else
            {
                sqls.push_back(head);
                sqls.back().append(tmp);
            }
            spos += tmp.size();
            range = size;
        }
    }
    tmp = state.substr(spos);
    if (!tmp.empty())
    {
        sqls.push_back(head);
        sqls.back().append(tmp);
    }
}

void CommitJob::ImportFromFile(pqxx::connection &qry)
{
    std::ifstream in(*_presult);
    if (!in)
    {
        //if failed change self to a error object
        std::error_code ec;
        std::filesystem::remove(*_presult, ec);
        _presult->clear();
        *_perr = "import failed!";
        CommitError(qry);
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
    CommitResult(qry);
}
