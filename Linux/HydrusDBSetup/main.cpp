
/****************************************************************************** 
 * 
 * 
 *  Copyright (c) 2019, Wenzhao Feng.
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

#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QString>
#include <string>
#include <tclap/CmdLine.h>
#include <QUuid>
#include <memory>
#include <future>
#include "databasesqlcommands.h"

bool _bFinished;

bool InitialDB(const std::string& strhost,const std::string& strdbname,const std::string& strusername,const std::string& strpassword,QSqlDatabase& db)
{
    db.setHostName(strhost.c_str());
    db.setUserName(strusername.c_str());
    db.setDatabaseName(strdbname.c_str());
    db.setPassword(strpassword.c_str());
    if(!db.open())
    {
        std::cout<<"Can't connect database!"<<std::endl;
        return false;
    }
    return true;
}

bool ParseCmdVars(int argc, char *argv[],QSqlDatabase& db,std::string& strnewdbname,int& tcount,int& gidcount)
{
    using namespace TCLAP;

    CmdLine cmd("This program is used to set up hydrus database.");

    ValueArg<std::string> dbname("D","databasename","the database name to connect",false,"postgres","string");
    ValueArg<std::string> newdbname("d","newdatabasename","the database name to set up",true,"","string");
    ValueArg<std::string> user("u","username","the username",true,"","string");
    ValueArg<std::string> password("p","password","the password",true,"","string");
    ValueArg<std::string> host("","hostname","the host name",false,"localhost","string");
    cmd.add(dbname);
    cmd.add(newdbname);
    cmd.add(user);
    cmd.add(password);
    cmd.add(host);

    ValueArg<int> tablecount("n","subtablecount","Set the number of sub-tables",false,1,"integer");
    ValueArg<int> gcount("N","gidcount","Set the number of soil columns stored in the subtable",false,50,"integer");
    cmd.add(tablecount);
    cmd.add(gcount);

    cmd.parse(argc,argv);

    if(!InitialDB(host.getValue(),dbname.getValue(),user.getValue(),password.getValue(),db))
    {
        return false;
    }

    tcount=tablecount.getValue();
    gidcount=gcount.getValue();
    strnewdbname=newdbname.getValue();
    return true;
}

std::string GetValidConName()
{
    return QUuid::createUuid().toString().toStdString();
}

bool ExecuteSqlCmd(QSqlQuery* qry,const std::string& sqlcmd)
{
    bool result=false;
    if(qry->exec(sqlcmd.c_str()))
    {
        result=true;
    }
    _bFinished=true;
    return result;
}

void DropDatabase(const std::string& sqlcmd,QSqlDatabase& db)
{
    QSqlQuery qry(db);
    ExecuteSqlCmd(&qry,sqlcmd);
}

bool CreateDatabase(const std::string& sqlcmd,QSqlDatabase& db,std::string& strnewdbname)
{
    _bFinished=false;
    bool result=false;
    std::string message="The DataBase "+strnewdbname;
    message+=" is creating, please wait ";
    std::cout<<message;
    QSqlQuery qry(db);

    try
    {
        std::future<bool> v=std::async(std::launch::async,ExecuteSqlCmd,&qry,sqlcmd);

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

bool CreateScheme(const std::string& sqlcmd,QSqlDatabase& db)
{
    _bFinished=false;
    bool result=false;
    std::string message="The Scheme of DataBase "+db.databaseName().toStdString();
    message+=" is creating, please wait ";
    std::cout<<message;
    QSqlQuery qry(db);

    try
    {
        std::future<bool> v=std::async(std::launch::async,ExecuteSqlCmd,&qry,sqlcmd);

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
        std::cout<<"Fail to Create Scheme of DataBase "<<db.databaseName().toStdString()
                <<"! Please consult the DBA!"<<std::endl;
    }
    else
    {
        std::cout<<"\r";
        std::cout<<"Scheme of DataBase "<<db.databaseName().toStdString()<<" has been created successfully!"
                <<std::endl;
    }
    return result;
}

int main(int argc, char *argv[])
{
    using namespace std;

    QCoreApplication _coreApp(argc,argv);
    
    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL",GetValidConName().c_str());
    string newdatabasename;
    int subtablecount;
    int gidcount;
    bool bPartition;

    if(!ParseCmdVars(argc,argv,db,newdatabasename,subtablecount,gidcount))
    {
        std::cout<<"\n*********The parameters given is not valid. Exit now.*********\n\n";
        db.close();
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
        pCmds.reset(new DataBaseSQLCommands(subtablecount,gidcount));
    }
    else
    {
        pCmds.reset(new DataBaseSQLCommands());
    }

    string sqldbcreatingcmd=pCmds->GetCreateDbSqlCommand(newdatabasename);

    if(CreateDatabase(sqldbcreatingcmd,db,newdatabasename))
    {
        string sqltablecreatingcmd=pCmds->GetCreateTablesSqlCommand();
        QSqlDatabase db2=QSqlDatabase::addDatabase("QPSQL",GetValidConName().c_str());
        db2.setHostName(db.hostName());
        db2.setDatabaseName(newdatabasename.c_str());
        db2.setUserName(db.userName());
        db2.setPassword(db.password());
        if(!db2.open())
        {
            std::cout<<"\nCan not open DataBase "<<newdatabasename<<std::endl;
            std::cout<<"Exist now! Please consult DBA!"<<std::endl;
        }
        else if(CreateScheme(sqltablecreatingcmd,db2))
        {
            std::cout<<"DataBase "<<newdatabasename
                    <<" has been completely created successfully."<<std::endl;
        }
        else
        {
            db2.close();
            std::string strsql="Drop database ";
            strsql+=newdatabasename;
            strsql+=";\n";
            DropDatabase(strsql,db);
            std::cout<<"DataBase "<<newdatabasename
                    <<" creation failed. Please consult DBA!"<<std::endl;
        }
        if(db2.isOpen())
        {
            db2.close();
        }
    }
    if(db.isOpen())
    {
        db.close();
    }
    return 0;
}
