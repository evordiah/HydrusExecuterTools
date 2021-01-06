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

#include <tclap/CmdLine.h>
#include <fstream>
#include <filesystem>
#include <regex>
#include <thread>
#include <future>
#include "taskcontroller.h"
#include "commandlineparser.h"
#include "tcp_client_inner.h"
#include "applicationparametermanager.h"
#include "pqdbconn.h"
#include "hydrusresource.h"
#include "asio.hpp"

CommandLineParser::CommandLineParser(int argc, char *argv[])
{
    Initial();
    switch (ParseIPandPort(argc, argv))
    {
    case -1:
        //missing -s or --server then let tclap show error message
        ParseServerCmd(argc, argv);
        _bSuccess = false;
        break;
    case 0:
        std::cout << "The ip address or port is not valid!" << std::endl;
        _bSuccess = false;
        break;
    case 1:
        ApplicationParameterManager::Instance().RunOnServer(false);
        ParseNodeCmd(argc, argv);
        break;
    case 2:
        ApplicationParameterManager::Instance().RunOnServer(true);
        ParseServerCmd(argc, argv);
        break;
    }
    Clear();
}

int CommandLineParser::ParseIPandPort(int argc, char *argv[])
{
    std::string strip;
    for (int i = 0; i < argc; ++i)
    {
        std::string temp = argv[i];
        Stringhelper sh(temp);
        if (0 == sh.compare("-s") || 0 == sh.compare("--server"))
        {
            if (i + 1 < argc)
            {
                strip = argv[i + 1];
                break;
            }
        }
    }
    if (strip.empty())
    {
        //missing -s or --server item,return -1
        return -1;
    }
    std::regex rex1("^(1?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                    "(1?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                    "(1?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                    "(1?\\d\\d?|2[0-4]\\d|25[0-5]):(\\d+)$");
    std::regex rex2("^:(\\d+)$");
    std::smatch mat;
    if (std::regex_match(strip, mat, rex1))
    {
        _serverip = mat[1].str();
        _serverip += ".";
        _serverip += mat[2].str();
        _serverip += ".";
        _serverip += mat[3].str();
        _serverip += ".";
        _serverip += mat[4].str();
        std::string strport = mat[5].str();
        _serverport = std::stoi(strport);
        ApplicationParameterManager::Instance().HostIP(_serverip);
        ApplicationParameterManager::Instance().HostPort(_serverport);
        return 1;
    }
    if (std::regex_match(strip, mat, rex2))
    {
        std::string strport = mat[1].str();
        _serverport = std::stoi(strport);
        ApplicationParameterManager::Instance().HostPort(_serverport);
        return 2;
    }
    return 0;
}

void CommandLineParser::ParseServerCmd(int argc, char *argv[])
{
    using namespace TCLAP;

    CmdLine cmd("This program is used to execute hydrus and import results into database.");

    ValueArg<std::string> dbname("D", "databasename", "the database name (required only for server)", true, _strbdname, "string");
    ValueArg<std::string> user("U", "username", "the username (required only for server)", true, _strdbusername, "string");
    ValueArg<std::string> password("P", "password", "the password (required only for server)", true, _strdbpassword, "string");
    ValueArg<std::string> host("H", "hostname", "the host name (required only for server), for example [127.0.0.1][:5432] or [localhost][:5432]", false, _strdbhost, "string");
    cmd.add(dbname);
    cmd.add(user);
    cmd.add(password);
    cmd.add(host);

    ValueArg<std::string> listfile("l", "listfile", "the file contained all the gids which will be executed,one gid perline (required only for server)", true, _listfile, "string");
    ValueArg<std::string> gid("g", "gid", "the ids to be executed, each id must be seperated by blank(s) (required only for server)", true, _gids, "string");
    ValueArg<std::string> whereclause("w", "where", "desiginate where clause to constraint the ids,please use standard sql where clause (required only for server)", true, _whereclause, "string");
    std::vector<Arg *> xorlist;
    xorlist.push_back(&listfile);
    xorlist.push_back(&gid);
    xorlist.push_back(&whereclause);
    cmd.xorAdd(xorlist);

    _appexecutingpath = Stringhelper(_appexecutingpath).replace("[target]", "server").str();
    ValueArg<std::string> dest("", "executingpath", "the folder for saving temp files and execute hydrus", false, _appexecutingpath, "string");
    cmd.add(dest);

    _logpath = Stringhelper(_logpath).replace("[target]", "server").str();
    ValueArg<std::string> logarg("", "log", "the file for recording errors  (required only for server)", false, _logpath, "string");
    cmd.add(logarg);

    ValueArg<unsigned int> threadcntarg("j", "thread_count", "set how many threads to run parally", false, _threadCnt, "integer");
    cmd.add(threadcntarg);

    ValueArg<std::string> hydrusexearg("e", "hydrus", "set executable hydrus file to run", false, _hydrusexecutablepath, "string");
    cmd.add(hydrusexearg);

    ValueArg<std::string> serverhostarg("s", "server", "server host and/or port,IP:Port(eg. 127.0.0.1:4000) for client, :Port(eg. :4000) for server", true, _serverip, "string");
    cmd.add(serverhostarg);

    ValueArg<unsigned int> waitingSecs("", "secs", "Set the number of seconds to wait for the hydrus to execute", false, _waitingSecs, "integer");
    cmd.add(waitingSecs);

    cmd.parse(argc, argv);
    //get values of db
    _strbdname = dbname.getValue();
    _strdbhost = host.getValue();
    _strdbusername = user.getValue();
    _strdbpassword = password.getValue();
    auto pos = _strdbhost.find(':');
    if (pos != std::string::npos)
    {
        _strdbport = _strdbhost.substr(pos + 1);
        _strdbhost.erase(pos);
    }
    Stringhelper sh(_strdbhost);
    _strdbhost = sh.trimmed().str();
    if (_strdbhost.empty())
    {
        _strdbhost = "localhost";
    }
    Stringhelper sp(_strdbport);
    _strdbport = sp.trimmed().str();
    if (_strdbport.empty())
    {
        _strdbport = "5432";
    }
    else
    {
        for (char c : _strdbport)
        {
            if (!std::isdigit(c))
            {
                std::cerr << _strdbport << " is not a valid port number!" << std::endl;
                _bSuccess = false;
                return;
            }
        }
    }
    CreateDbonServer();
    if (!_bSuccess)
    {
        return;
    }
    //get values about gids
    if (whereclause.isSet())
    {
        _whereclause = whereclause.getValue();
        GetIdsFromDB();
    }
    if (listfile.isSet())
    {
        _listfile = listfile.getValue();
        GetIdsFromFile();
    }
    if (gid.isSet())
    {
        _gids = gid.getValue();
        GetIdsFromList();
    }
    if (!_bSuccess)
    {
        return;
    }
    _appexecutingpath = dest.getValue();
    std::filesystem::path p = _appexecutingpath;
    if (!std::filesystem::exists(p))
    {
        if (!std::filesystem::create_directories(p))
        {
            _bSuccess = false;
            std::cerr << "Can not find or create path " << _appexecutingpath << std::endl;
            return;
        }
    }
    ApplicationParameterManager::Instance().HydursExecutingPath(_appexecutingpath);

    _logpath = logarg.getValue();
    ApplicationParameterManager::Instance().LogFile(_logpath);

    _threadCnt = threadcntarg.getValue();

    ApplicationParameterManager::Instance().ThreadCount(_threadCnt);

    _hydrusexecutablepath = hydrusexearg.getValue();
    PrepareHydrus();
    if (!_bSuccess)
    {
        std::cerr << "Can not find hydrus executable." << std::endl;
        return;
    }
    _waitingSecs = waitingSecs.getValue();
    ApplicationParameterManager::Instance().WaitSeconds(_waitingSecs);
}

void CommandLineParser::ParseNodeCmd(int argc, char *argv[])
{
    using namespace TCLAP;

    CmdLine cmd("This program is used to execute hydrus and import results into database.");

    _appexecutingpath = Stringhelper(_appexecutingpath).replace("[target]", "node").str();
    ValueArg<std::string> dest("", "executingpath", "the folder for saving temp files and execute hydrus", false, _appexecutingpath, "string");
    cmd.add(dest);

    ValueArg<unsigned int> threadcntarg("j", "thread_count", "set how many threads to run parally", false, _threadCnt, "integer");
    cmd.add(threadcntarg);

    ValueArg<std::string> hydrusexearg("e", "hydrus", "set executable hydrus file to run", false, _hydrusexecutablepath, "string");
    cmd.add(hydrusexearg);

    ValueArg<std::string> serverhostarg("s", "server", "server host and/or port,IP:Port(eg. 127.0.0.1:4000) for client, :Port(eg. :4000) for server", true, _serverip, "string");
    cmd.add(serverhostarg);

    ValueArg<unsigned int> waitingSecs("", "secs", "Set the number of seconds to wait for the hydrus to execute", false, _waitingSecs, "integer");
    cmd.add(waitingSecs);

    cmd.parse(argc, argv);

    _appexecutingpath = dest.getValue();
    std::filesystem::path p = _appexecutingpath;
    if (!std::filesystem::exists(p))
    {
        if (!std::filesystem::create_directories(p))
        {
            _bSuccess = false;
            std::cerr << "Can not find or create path " << _appexecutingpath << std::endl;
            return;
        }
    }
    ApplicationParameterManager::Instance().HydursExecutingPath(_appexecutingpath);

    _threadCnt = threadcntarg.getValue();
    if (_threadCnt == 0)
    {
        _threadCnt = std::thread::hardware_concurrency();
        if (_threadCnt == 0)
        {
            _threadCnt = 8;
        }
    }

    ApplicationParameterManager::Instance().ThreadCount(_threadCnt);

    _hydrusexecutablepath = hydrusexearg.getValue();
    PrepareHydrus();
    if (!_bSuccess)
    {
        std::future<bool> ret = std::async(&CommandLineParser::GetHydrusFromServer, this);
        _bSuccess = ret.get();
    }
    if (!_bSuccess)
    {
        std::cerr << "Can not get hydrus executable. " << std::endl;
        return;
    }
    _waitingSecs = waitingSecs.getValue();
    ApplicationParameterManager::Instance().WaitSeconds(_waitingSecs);
    try
    {
        CreateDbOnNode();
    }
    catch (std::string &err)
    {
        _bSuccess = false;
        std::cout << err << std::endl;
        return;
    }
}

void CommandLineParser::CreateDbonServer()
{
    pqDBConn *pDB = DBConnManager::GetInstance().GetConnection();
    if (!pDB)
    {
        _bSuccess = false;
        return;
    }
    pDB->HostName(_strdbhost);
    pDB->Port(_strdbport);
    pDB->DBName(_strbdname);
    pDB->UserName(_strdbusername);
    pDB->PassWord(_strdbpassword);
    _bSuccess = pDB->GetConn() != nullptr;
}

void CommandLineParser::CreateDbOnNode()
{
    try
    {
        pqDBConn *pDB = DBConnManager::GetInstance().GetConnection();
        if (!pDB)
        {
            _bSuccess = false;
            return;
        }
        std::string constr = GetConnectionStringFromServer();
        if (!constr.empty())
        {
            std::stringstream instream(constr);
            instream >> _strdbhost >> _strbdname >> _strdbusername >> _strdbpassword >> _strdbport;
            auto pos = _strdbhost.find('=');
            _strdbhost = _strdbhost.substr(pos + 1);
            pos = _strbdname.find('=');
            _strbdname = _strbdname.substr(pos + 1);
            pos = _strdbusername.find('=');
            _strdbusername = _strdbusername.substr(pos + 1);
            pos = _strdbpassword.find('=');
            _strdbpassword = _strdbpassword.substr(pos + 1);
            pos = _strdbport.find('=');
            _strdbport = _strdbport.substr(pos + 1);
        }
        else
        {
            _strdbhost = "NONE";
            _strdbusername = "NONE";
            _strdbpassword = "NONE";
            _strdbport = "NONE";
            _strbdname = "NONE";
        }
        if (_strdbhost == "localhost")
        {
            _strdbhost = _serverip;
        }       
        pDB->HostName(_strdbhost);
        pDB->Port(_strdbport);
        pDB->DBName(_strbdname);
        pDB->UserName(_strdbusername);
        pDB->PassWord(_strdbpassword);
    }
    catch (std::exception &e)
    {
        throw std::string(e.what());
    }
}

void CommandLineParser::GetIdsFromDB()
{
    pqDBConn *pDB = DBConnManager::GetInstance().GetConnection();
    pqxx::connection *pConn = nullptr;
    if (pDB && nullptr != (pConn = pDB->GetConn()))
    {
        std::stringstream strbld;
        strbld << "select gid from selector where status='todo' and (" << _whereclause << ");";
        try
        {
            pqxx::work w(*pConn);
            auto rs = w.exec(strbld.str());
            w.commit();
            for (const auto &r : rs)
            {
                TaskController::GetController().AddNewTask(r[0].as<int>());
            }
        }
        catch (std::exception &e)
        {
            _bSuccess = false;
            std::cerr << e.what() << std::endl;
        }
    }
    else
    {
        _bSuccess = false;
    }
    if (!_bSuccess)
    {
        std::cerr << "Can't get valid ids from database! May be you give invalid where criteria!" << std::endl;
    }
}

void CommandLineParser::GetIdsFromList()
{
    std::string lst = Stringhelper(_gids).simplified().str();
    if (lst.empty())
    {
        _bSuccess = false;
        std::cerr << "Can't get valid ids because gid list is empty!" << std::endl;
        return;
    }
    std::stringstream strbld(lst);
    std::stringstream strbldsql;
    strbldsql << "select gid from selector where status='todo' and gid in (";
    std::string sv;
    while (std::getline(strbld, sv, ' '))
    {
        try
        {
            strbldsql << stoi(sv) << ",";
        }
        catch (...)
        {
        }
    }
    std::string sqlstm = strbldsql.str();
    sqlstm.back() = ')';
    sqlstm.push_back(';');

    pqDBConn *pDB = DBConnManager::GetInstance().GetConnection();
    pqxx::connection *pConn = nullptr;
    if (pDB && nullptr != (pConn = pDB->GetConn()))
    {
        try
        {
            pqxx::work w(*pConn);
            auto rs = w.exec(sqlstm);
            w.commit();
            for (const auto &r : rs)
            {
                TaskController::GetController().AddNewTask(r[0].as<int>());
            }
        }
        catch (std::exception &e)
        {
            _bSuccess = false;
            std::cerr << e.what() << std::endl;
        }
    }
    else
    {
        _bSuccess = false;
    }
    if (!_bSuccess)
    {
        std::cerr << "Can't get valid ids from database! May be you give invalid gid list!" << std::endl;
    }
}

void CommandLineParser::GetIdsFromFile()
{
    if (!std::filesystem::exists(_listfile))
    {
        _bSuccess = false;
        std::cerr << _listfile << " is not a valid file!" << std::endl;
        return;
    }

    std::stringstream strbldsql;
    strbldsql << "select gid from selector where status='todo' and gid in (";

    std::string line;
    std::ifstream in(_listfile);
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;
        try
        {
            strbldsql << stoi(line) << ",";
        }
        catch (...)
        {
        }
    }
    in.close();
    std::string sqlstm = strbldsql.str();
    sqlstm.back() = ')';
    sqlstm.push_back(';');

    pqDBConn *pDB = DBConnManager::GetInstance().GetConnection();
    pqxx::connection *pConn = nullptr;
    if (pDB && nullptr != (pConn = pDB->GetConn()))
    {
        try
        {
            pqxx::work w(*pConn);
            auto rs = w.exec(sqlstm);
            w.commit();
            for (const auto &r : rs)
            {
                TaskController::GetController().AddNewTask(r[0].as<int>());
            }
        }
        catch (std::exception &e)
        {
            _bSuccess = false;
            std::cerr << e.what() << std::endl;
        }
    }
    else
    {
        _bSuccess = false;
    }
    if (!_bSuccess)
    {
        std::cerr << "Can't get valid ids from database! May be you give invalid id list in file!" << std::endl;
    }
}

void CommandLineParser::PrepareHydrus()
{
    std::filesystem::path sourcepath = _hydrusexecutablepath;
    std::filesystem::path destexepath = _appexecutingpath;
    auto hydrusExe = std::filesystem::absolute(destexepath / "hydrus.exe");
    if (std::filesystem::exists(sourcepath))
    {
        _bSuccess = true;
        if (!std::filesystem::exists(hydrusExe))
        {
            try
            {
                std::filesystem::copy(sourcepath, hydrusExe);
            }
            catch (...)
            {
                _bSuccess = false;
            }
        }
    }
    else
    {
        std::ofstream out(hydrusExe.string(), std::ios_base::binary);
        out.write(reinterpret_cast<const char *>(hydrus_resource), hydrus_resource_len);
        _bSuccess = true;
    }
}

std::string CommandLineParser::GetConnectionStringFromServer()
{
    //get connectionstring from server
    asio::io_context io;
    auto tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::requireConnectionString);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(_serverip), _serverport);
    tcp->start(ep);
    io.run();
    if (tcp->State() == tcp_client_inner::enumTcpSocketState::FatalError)
    {
        throw std::string("Can not connect to host");
    }
    return tcp->Data();
}

bool CommandLineParser::GetHydrusFromServer()
{
    std::filesystem::path destexepath = _appexecutingpath;
    std::string hydrusExe = std::filesystem::absolute(destexepath / "hydrus.exe").string();
    asio::io_context io;
    auto tcp = tcp_client_inner::Tcp_Client::NewConnection(io, tcp_client_inner::enumRequire::requireHydrus);
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(_serverip), _serverport);
    tcp->UserData(hydrusExe);
    tcp->start(ep);
    io.run();
    return tcp->State() == tcp_client_inner::enumTcpSocketState::Succeed;
}

void CommandLineParser::Initial()
{
    _strbdname = "hydrusdb";
    _strdbhost = "localhost:5432";
    _strdbusername = "postgres";
    _strdbpassword = "";
    _strdbport = "5432";
    _serverip = "127.0.0.1";
    _serverport = 65535;

#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
    _appexecutingpath = std::filesystem::absolute(std::filesystem::current_path() / "executing_[target]").string();
    _logpath = std::filesystem::absolute(std::filesystem::current_path() / "hydrusexecutingerrlog_[target].log").string();
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
    _appexecutingpath = "/tmp/executing_[target]";
    _logpath = "/tmp/hydrusexecutingerrlog_[target].log";
#else
#error unknown os
#endif
    _hydrusexecutablepath = "";
    _threadCnt = std::thread::hardware_concurrency();
    _waitingSecs = 0;
    //gid options
    _listfile = "";
    _gids = "";
    _whereclause = "";
    _bSuccess = true;
}

void CommandLineParser::Clear()
{
    _strbdname.clear();
    _strdbhost.clear();
    _strdbusername.clear();
    _strdbpassword.clear();
    _strdbport.clear();
    _serverip.clear();
    _appexecutingpath.clear();
    _logpath.clear();
    _hydrusexecutablepath.clear();
    _listfile.clear();
    _gids.clear();
    _whereclause.clear();
}
