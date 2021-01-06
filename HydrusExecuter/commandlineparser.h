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

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>

class CommandLineParser
{
public:
    CommandLineParser(int argc, char *argv[]);
    CommandLineParser(const CommandLineParser& rhs)=delete;
    CommandLineParser(CommandLineParser&& rhs)=delete;
    CommandLineParser& operator=(const CommandLineParser& rhs)=delete;
    CommandLineParser& operator=(CommandLineParser&& rhs)=delete;
    ~CommandLineParser()=default;

    operator bool() const
    {
        return _bSuccess;
    }

private:
    void Initial();
    void Clear();
    int ParseIPandPort(int argc, char *argv[]);
    void ParseServerCmd(int argc, char *argv[]);
    void ParseNodeCmd(int argc, char *argv[]);
    void CreateDbonServer();
    void CreateDbOnNode();
    void GetIdsFromDB();
    void GetIdsFromList();
    void GetIdsFromFile();
    void PrepareHydrus();
    std::string GetConnectionStringFromServer();
    bool GetHydrusFromServer();
private:
    //database
    std::string _strbdname;
    std::string _strdbhost;
    std::string _strdbusername;
    std::string _strdbpassword;
    std::string _strdbport;
    //ip address of server
    std::string _serverip;
    unsigned short _serverport;
    //app executing temp path
    std::string _appexecutingpath;
    //log file
    std::string _logpath;
    //hydrus executable path
    std::string _hydrusexecutablepath;
    //number of thread
    unsigned int _threadCnt;
    //waiting untile hydrus finished unit:second
    unsigned int _waitingSecs;
    //gid options
    std::string _listfile;
    std::string _gids;
    std::string _whereclause;
    bool _bSuccess;
};

#endif // COMMANDLINEPARSER_H
