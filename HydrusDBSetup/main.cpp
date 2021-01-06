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

#include <string>
#include <tclap/CmdLine.h>
#include <memory>
#include <future>
#include "Stringhelper.h"
#include "databasesqlcommands.h"
#include "pqdbconn.h"

bool _bFinished;

bool ParseCmdVars(int argc, char *argv[],pqDBConn& conn,
                  std::string& strnewdbname,int& tcount,int& gidcount)
{
    using namespace TCLAP;

    CmdLine cmd("This program is used to set up hydrus database.");

    ValueArg<std::string> dbname("d","databasename","the database name to connect",false,"postgres","string");
    ValueArg<std::string> newdbname("D","newdatabasename","the database name to set up",true,"","string");
    ValueArg<std::string> user("U","username","the username",true,"","string");
    ValueArg<std::string> password("P","password","the password",true,"","string");
    ValueArg<std::string> host("H","hostname","the host name, for example [127.0.0.1][:5432] or [localhost][:5432]",false,"localhost:5432","string");
    cmd.add(dbname);
    cmd.add(newdbname);
    cmd.add(user);
    cmd.add(password);
    cmd.add(host);

    ValueArg<int> tablecount("N","subtablecount","Set the number of sub-tables",false,1,"integer");
    ValueArg<int> gcount("n","gidcount","Set the number of soil columns stored in the subtable",false,50,"integer");
    cmd.add(tablecount);
    cmd.add(gcount);

    cmd.parse(argc,argv);
    std::string hostname=host.getValue();
    std::string port;
    auto pos=hostname.find(':');
    if(pos!=std::string::npos)
    {
        port=hostname.substr(pos+1);
        hostname.erase(pos);
    }
    Stringhelper sh(hostname);
    hostname=sh.trimmed().str();
    if(hostname.empty())
    {
        hostname="localhost";
    }
    Stringhelper sp(port);
    port=sp.trimmed().str();
    if(port.empty())
    {
        port="5432";
    }
    else
    {
        for(char c:port)
        {
            if(!std::isdigit(c))
            {
                std::cerr<<port<<" is not a valid port number!"<<std::endl;
                return false;
            }
        }
    }
    conn.DBName(dbname.getValue());
    conn.UserName(user.getValue());
    conn.HostName(hostname);
    conn.Port(port);
    conn.PassWord(password.getValue());
    if(!conn.GetConn())
    {
        std::cout<<"Can't connect database!"<<std::endl;
        return false;
    }
    tcount=tablecount.getValue();
    gidcount=gcount.getValue();
    strnewdbname=newdbname.getValue();
    return true;
}

bool ExecuteSqlCmd(pqxx::connection* conn,const std::string& sqlcmd)
{
    bool result=false;
    if(!conn)
    {
        return result;
    }
    try
    {
        pqxx::nontransaction work(*conn);
        work.exec(sqlcmd);
        result=true;
    }
    catch (std::exception& e)
    {
        std::cerr<<e.what()<<std::endl;
    }
    _bFinished=true;
    return result;
}

void DropDatabase(const std::string& sqlcmd,pqxx::connection* conn)
{
    ExecuteSqlCmd(conn,sqlcmd);
}

bool CreateDatabase(const std::string& sqlcmd,pqxx::connection* conn,std::string& strnewdbname)
{
    _bFinished=false;
    bool result=false;
    std::string message="The DataBase "+strnewdbname;
    message+=" is creating, please wait ";
    std::cout<<message;

    try
    {
        std::future<bool> v=std::async(std::launch::async,ExecuteSqlCmd,conn,sqlcmd);

        int i=0;
        while (!_bFinished)
        {
            char label[] ={'\\','|','/','_'};
            std::cout<<'\r';
            std::cout<<message<<label[i++];
            std::this_thread::sleep_for(std::chrono::microseconds(30000));
            if(i==4)
            {
                i=0;
            }
        }
        result=v.get();
    }
    catch(...)
    {
        result=false;
    }
    if(!result)
    {
        std::cout<<"\r";
        std::cout<<"Fail to Create DataBase "<<strnewdbname
                <<"! Please consult the DBA!"<<std::endl;
    }
    else
    {
        std::cout<<"\r";
        std::cout<<"DataBase "<<strnewdbname<<" has been created successfully!"
                <<std::endl;
    }
    return result;
}

bool CreateScheme(const std::string& sqlcmd,pqxx::connection* conn)
{
    _bFinished=false;
    bool result=false;
    std::string message="The Scheme of DataBase "+std::string(conn->dbname());
    message+=" is creating, please wait ";
    std::cout<<message;

    try
    {
        std::future<bool> v=std::async(std::launch::async,ExecuteSqlCmd,conn,sqlcmd);

        int i=0;
        while (!_bFinished)
        {
            char label[] ={'\\','|','/','_','\\','|','/','_'};
            std::cout<<'\r';
            std::cout<<message<<label[i++];
            std::this_thread::sleep_for(std::chrono::microseconds(30000));
            if(i==8)
            {
                i=0;
            }
        }
        result=v.get();
    }
    catch(...)
    {
        result=false;
    }
    if(!result)
    {
        std::cout<<"\r";
        std::cout<<"Fail to Create Scheme of DataBase "<<std::string(conn->dbname())
                <<"! Please consult the DBA!"<<std::endl;
    }
    else
    {
        std::cout<<"\r";
        std::cout<<"Scheme of DataBase "<<std::string(conn->dbname())<<" has been created successfully!"
                <<std::endl;
    }
    return result;
}

int main(int argc, char *argv[])
{
    using namespace std;

    string newdatabasename;
    int subtablecount;
    int gidcount;
    bool bPartition = false;
    pqDBConn& conn=*DBConnManager::GetInstance().GetConnection();
    if(!ParseCmdVars(argc,argv,conn,newdatabasename,subtablecount,gidcount))
    {
        std::cout<<"\n*********The parameters given is not valid. Exit now.*********\n\n";
        return 0;
    }

    if(subtablecount>1)
    {
        bPartition=true;
        if(gidcount<=0)
        {
            gidcount=50;
        }
    }


    unique_ptr<DataBaseSQLCommands> pCmds;

    if(bPartition)
    {
        pCmds = std::make_unique<DataBaseSQLCommands>(subtablecount,gidcount);
    }
    else
    {
        pCmds = std::make_unique<DataBaseSQLCommands>();
    }

    string sqldbcreatingcmd=pCmds->GetCreateDbSqlCommand(newdatabasename);

    if(CreateDatabase(sqldbcreatingcmd,conn.GetConn(),newdatabasename))
    {
        string sqltablecreatingcmd=pCmds->GetCreateTablesSqlCommand();
        std::string oldDbName=conn.DBName();
        conn.DBName(newdatabasename);

        if(!conn.GetConn())
        {
            std::cout<<"\nCan not open DataBase "<<newdatabasename<<std::endl;
            std::cout<<"Exist now! Please consult DBA!"<<std::endl;
        }
        else if(CreateScheme(sqltablecreatingcmd,conn.GetConn()))
        {
            std::cout<<"DataBase "<<newdatabasename
                    <<" has been completely created successfully."<<std::endl;
        }
        else
        {
            conn.DBName(oldDbName);
            std::string strsql="Drop database ";
            strsql+=newdatabasename;
            strsql+=";\n";
            DropDatabase(strsql,conn.GetConn());
            std::cout<<"DataBase "<<newdatabasename
                    <<" creation failed. Please consult DBA!"<<std::endl;
        }
    }
    return 0;
}
