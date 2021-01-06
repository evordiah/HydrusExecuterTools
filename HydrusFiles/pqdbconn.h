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

#ifndef PQDBCONN_H
#define PQDBCONN_H

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <pqxx/pqxx>
#include "Stringhelper.h"

class pqDBConn
{
    friend class DBConnManager;

public:
    pqDBConn(const pqDBConn &rhs) = delete;
    pqDBConn(pqDBConn &&rhs) = delete;
    pqDBConn &operator=(const pqDBConn &rhs) = delete;
    pqDBConn &operator=(pqDBConn &&rhs) = delete;
    std::string DBName()
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        return _dbname;
    }
    void DBName(const std::string &v)
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        if (v != _dbname)
        {
            _dbname = v;
            _changed = true;
        }
    }
    std::string UserName()
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        return _username;
    }
    void UserName(const std::string &v)
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        if (v != _username)
        {
            _username = v;
            _changed = true;
        }
    }
    std::string HostName()
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        return _hostname;
    }
    void HostName(const std::string &v)
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        if (v != _hostname)
        {
            _hostname = v;
            _changed = true;
        }
    }
    std::string Port()
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        return _port;
    }
    void Port(const std::string &v)
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        if (v != _port)
        {
            _port = v;
            _changed = true;
        }
    }
    std::string PassWord()
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        return _passwd;
    }
    void PassWord(const std::string &v)
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        if (v != _passwd)
        {
            _passwd = v;
            _changed = true;
        }
    }

    std::string GetConnectionString()
    {
        Stringhelper sh("host=%1 dbname=%2 user=%3 password=%4 port=%5");
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        return sh.arg(_hostname).arg(_dbname).arg(_username).arg(_passwd).arg(_port).str();
    }

    pqxx::connection *GetConn()
    {
        std::lock_guard<std::recursive_mutex> lk(_mtx);
        if (!_pConn || _changed)
        {
            try
            {
                _pConn = std::make_unique<pqxx::connection>(GetConnectionString());
                _changed = false;
            }
            catch (...)
            {
                _pConn = nullptr;
            }
        }
        return _pConn.get();
    }

    std::shared_ptr<pqDBConn> Clone()
    {
        auto pnt = new pqDBConn();
        {
            std::lock_guard<std::recursive_mutex> lk(_mtx);
            pnt->_dbname = _dbname;
            pnt->_hostname = _hostname;
            pnt->_passwd = _passwd;
            pnt->_port = _port;
            pnt->_username = _username;
        }
        pnt->GetConn();
        return std::shared_ptr<pqDBConn>(pnt);
    }

private:
    pqDBConn()
    {
        _changed = false;
    }

    std::string _dbname;
    std::string _username;
    std::string _hostname;
    std::string _port;
    std::string _passwd;
    bool _changed;
    std::unique_ptr<pqxx::connection> _pConn;
    std::recursive_mutex _mtx;
};

class DBConnManager
{
public:
    static DBConnManager &GetInstance()
    {
        if (!_pDBConnManager)
        {
            _pDBConnManager.reset(new DBConnManager());
        }
        return *_pDBConnManager;
    }

    pqDBConn *MakeConnection(const std::string &conname, const std::string &host,
                             const std::string &port, const std::string &dbname,
                             const std::string &user, const std::string &password)
    {
        pqDBConn *pConn = nullptr;
        {
            std::lock_guard<std::recursive_mutex> lg(_mtx);
            if (_ConnList.find(conname) != _ConnList.end())
            {
                pConn = _ConnList[conname].get();
            }
            else
            {
                pConn = new pqDBConn();
                _ConnList.insert(std::make_pair(conname, std::shared_ptr<pqDBConn>(pConn)));
            }
        }
        pConn->HostName(host);
        pConn->Port(port);
        pConn->DBName(dbname);
        pConn->UserName(user);
        pConn->PassWord(password);
        return pConn;
    }

    pqDBConn *MakeConnection(const std::string &conname, std::shared_ptr<pqDBConn> Conn)
    {
        std::lock_guard<std::recursive_mutex> lg(_mtx);
        auto it = _ConnList.find(conname);
        if (it != _ConnList.end())
        {
            _ConnList.erase(it);
        }
        _ConnList.insert(std::make_pair(conname, Conn));
        return Conn.get();
    }

    void RemoveConnection(const std::string &connname)
    {
        std::lock_guard<std::recursive_mutex> lg(_mtx);
        auto it = _ConnList.find(connname);
        if (it != _ConnList.end())
        {
            _ConnList.erase(it);
        }
    }

    pqDBConn *GetConnection(const std::string &connname = "default")
    {
        std::lock_guard<std::recursive_mutex> lg(_mtx);
        if (_ConnList.find(connname) != _ConnList.end())
        {
            return _ConnList[connname].get();
        }
        auto pConn = new pqDBConn();
        _ConnList.insert(std::make_pair(connname, std::shared_ptr<pqDBConn>(pConn)));
        return pConn;
    }

    pqDBConn *GetorCloneConnection(const std::string &connname,const std::string &templateconname="default")
    {
        std::lock_guard<std::recursive_mutex> lg(_mtx);
        if (_ConnList.find(connname) != _ConnList.end())
        {
            return _ConnList[connname].get();
        }
        if (_ConnList.find(templateconname) != _ConnList.end())
        {
            auto pConn=_ConnList[templateconname]->Clone();
            _ConnList.insert(std::make_pair(connname, pConn));
            return pConn.get();
        }
        return nullptr;
    }

private:
    DBConnManager()
    {
        auto pConn = new pqDBConn();
        _ConnList.insert(std::make_pair("default", std::shared_ptr<pqDBConn>(pConn)));
    }
    std::recursive_mutex _mtx;
    std::map<std::string, std::shared_ptr<pqDBConn>> _ConnList;
    static std::unique_ptr<DBConnManager> _pDBConnManager;
};

#endif // PQDBCONN_H
