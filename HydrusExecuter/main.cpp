
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

#include <QCoreApplication>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <string>
#include <tclap/CmdLine.h>
#include <iostream>
#include <fstream>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <thread>
#include <future>
#include "taskcontroller.h"

bool InitialDB(const std::string& strhost,const std::string& strdbname,const std::string& strusername,const std::string& strpassword,QSqlDatabase& db)
{
    db.setHostName(strhost.c_str());
    db.setUserName(strusername.c_str());
    db.setDatabaseName(strdbname.c_str());
    db.setPassword(strpassword.c_str());
    if(!db.open())
    {
        std::cerr<<"Can't connect database!"<<std::endl;
        return false;
    }
    return true;
}

bool GetIdsFromDB(QSqlDatabase& db,const std::string& whereclause)
{
    QSqlQuery qry(db);
    std::stringstream strbld;
    strbld<<"select gid from selector where status='todo' and ("<<whereclause<<");";
    if(qry.exec(strbld.str().c_str()))
    {
        while(qry.next())
        {
            TaskController::GetController().AddNewTask(qry.value(0).toInt());
        }
    }
    else
    {
        std::cerr<<"Can't get valid ids from database! May be you give invalid where criteria!"<<std::endl;
        return false;
    }
    return true;
}

bool GetIdsFromList(QSqlDatabase& db,std::string& idlist)
{
    std::string lst=QString(idlist.c_str()).simplified().toStdString();
    if(lst.empty())
    {
        return false;
    }
    std::stringstream strbld(lst);
    std::stringstream strbldsql;
    strbldsql<<"select gid from selector where status='todo' and gid in (";
    std::string sv;
    while(std::getline(strbld,sv,' '))
    {
        try
        {
            strbldsql << stoi(sv) << ",";
        }
        catch(...)
        {
        }
    }
    std::string sqlstm = strbldsql.str();
    sqlstm.back() = ')';
    sqlstm.push_back(';');
    QSqlQuery qry(db);
    if (qry.exec(sqlstm.c_str()))
    {
        while (qry.next())
        {
            TaskController::GetController().AddNewTask(qry.value(0).toInt());
        }
    }
    else
    {
        std::cerr<<"Can't get valid ids from list! May be you give invalid gid list!"<<std::endl;
        return false;
    }
    return true;
}

bool GetIdsFromFile(QSqlDatabase& db, std::string& filename)
{
    if(!QFileInfo::exists(filename.c_str()))
    {
        std::cerr<<filename<<" is not a valid file!"<<std::endl;
        return false;
    }

    std::stringstream strbldsql;
    strbldsql << "select gid from selector where status='todo' and gid in (";

    std::string line;
    std::ifstream in(filename);
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
    QSqlQuery qry(db);
    if (qry.exec(sqlstm.c_str()))
    {
        while (qry.next())
        {
            TaskController::GetController().AddNewTask(qry.value(0).toInt());
        }
    }
    else
    {
        std::cerr<<"Can't get valid ids from file! May be you give invalid gids in the file, or the format of the file is not valid!"<<std::endl;
        return false;
    }
    return true;
}

bool ParseCmdVars(int argc, char *argv[],std::string& strhydrusexefile,
                  std::string& strexepath,unsigned int& threadcnt,QSqlDatabase& db,bool& bEven)
{
    using namespace TCLAP;

    CmdLine cmd("This program is used to execute hydrus and import results into database.");

    ValueArg<std::string> dbname("D","databasename","the database name",true,"","string");
    ValueArg<std::string> user("U","username","the username",true,"","string");
    ValueArg<std::string> password("P","password","the password",true,"","string");
    ValueArg<std::string> host("H","hostname","the host name",false,"localhost","string");
    cmd.add(dbname);
    cmd.add(user);
    cmd.add(password);
    cmd.add(host);

    ValueArg<std::string> listfile("l","listfile","the file contained all the gids which will be executed,one gid perline",true,"","string");
    ValueArg<std::string> gid("g","gid","the ids to be executed, each id must be seperated by blank(s)",true,"","string");
    ValueArg<std::string> whereclause("w","where","desiginate where clause to constraint the ids,please use standard sql where clause",true,"","string");
    std::vector<Arg*>  xorlist;
    xorlist.push_back(&listfile);
    xorlist.push_back(&gid);
    xorlist.push_back(&whereclause);
    cmd.xorAdd(xorlist);

#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
    QDir currentpath= QCoreApplication::applicationDirPath();
    std::string exepath = QDir::toNativeSeparators(currentpath.absoluteFilePath("executing")).toStdString();
    std::string logpath = QDir::toNativeSeparators(currentpath.absoluteFilePath("hydrusexecutingerrlog.log")).toStdString();
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
    std::string exepath="/tmp/executing";
    std::string logpath="/tmp/hydrusexecutingerrlog.log";
#else
#   error unknown os
#endif

    ValueArg<std::string> dest("","exepath","the folder for saving temp files and execute hydrus",false,exepath,"string");
    cmd.add(dest);

    ValueArg<std::string> logarg("","log","the file for recording errors ",false,logpath,"string");
    cmd.add(logarg);

    ValueArg<unsigned int> threadcntarg("j","thread_count","set how many threads to run parally",false,std::thread::hardware_concurrency(),"integer");
    cmd.add(threadcntarg);

    ValueArg<std::string> hydrusexearg("e","hydrus","set executable hydrus file to run",true,"","string");
    cmd.add(hydrusexearg);

    SwitchArg exestrategy("","evenly","Set a parallel execution policy, set this parameter, resulting in the average assignment of calculated tasks",false);
    cmd.add(exestrategy);

    ValueArg<int> waitingSecs("","secs","Set the number of seconds to wait for the hydrus to execute",false,0,"integer");
    cmd.add(waitingSecs);

    cmd.parse(argc,argv);

    if(!InitialDB(host.getValue(),dbname.getValue(),user.getValue(),password.getValue(),db))
    {
        return false;
    }

    bEven=exestrategy.getValue();

    threadcnt=threadcntarg.getValue();
    if(threadcnt<=0)
    {
        threadcnt=std::thread::hardware_concurrency();
        if(threadcnt==0)
        {
            threadcnt=8;
        }
    }

    strexepath=dest.getValue();
    QDir p(strexepath.c_str());
    if(!p.exists())
    {
        if(!p.mkpath(strexepath.c_str()))
        {
            std::cerr<<"Can not find or create path "<<strexepath<<std::endl;
            return false;
        }
    }

    strhydrusexefile=hydrusexearg.getValue();
    if(!QFileInfo::exists(strhydrusexefile.c_str()))
    {
        std::cerr<<"Can not find "<<strhydrusexefile<<std::endl;
        return false;
    }

    TaskController::GetController().WaitHydrusForSeconds(waitingSecs.getValue());

    TaskController::GetController().LogFile(logarg.getValue());

    if(whereclause.isSet())
    {
        return GetIdsFromDB(db,whereclause.getValue());
    }
    else if(listfile.isSet())
    {
        return GetIdsFromFile(db,listfile.getValue());
    }
    else
    {
        return GetIdsFromList(db,gid.getValue());
    }
}

bool ProviderHydrusExe(const std::string& hydrusexefile,const std::string& exepath)
{
    bool result=true;

    QDir qdexepath(exepath.c_str());
    QString hydrusExe=qdexepath.absoluteFilePath("hydrus.exe");
    if(!QFile::exists(hydrusExe))
    {
        try
        {
            QFile::copy(hydrusexefile.c_str(),hydrusExe);
        }
        catch(...)
        {
            result=false;
        }
    }
    return result;
}

bool _bFinished;

void ExecuteOperation(void* pdb,void* ppath,void *pbEven)
{
    QSqlDatabase& db =*static_cast<QSqlDatabase*>(pdb);
    std::string& path=*static_cast<std::string*>(ppath);
    bool bEven=*static_cast<bool*>(pbEven);
    if(!bEven)
    {
        TaskController::GetController().Run(db,path);
    }
    else
    {
        TaskController::GetController().RunEvenly(db,path);
    }
    _bFinished=true;
}

int main(int argc, char *argv[])
{
    using namespace std;
    QCoreApplication _coreApp(argc,argv);
    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL",QUuid::createUuid().toString());
    string exepath,hydrusexefile;
    unsigned int igiventhreadcnt;
    bool bEven;
    if(!ParseCmdVars(argc,argv,hydrusexefile,exepath,igiventhreadcnt,db,bEven))
    {
        std::cout<<"\n*********The parameters given is not valid. Exit now.*********\n\n";
        return 0;
    }

    if(!TaskController::GetController().ShouldRun())
    {
        std::cout<<"\n*********There no valid gids. Exit now.*********\n\n";
        return 0;
    }

    TaskController::GetController().ThreadCount(igiventhreadcnt);

    if(!ProviderHydrusExe(hydrusexefile,exepath))
    {
        std::cout<<"\n*********The hydrus executable file is not OK. Exit now.*********\n\n";
        return 0;
    }

    _bFinished=false;
    try
    {
        std::string message="Operation is exectuing, please wait  ";
        std::future<void> v=std::async(std::launch::async,ExecuteOperation,reinterpret_cast<void*>(&db),
                                       reinterpret_cast<void*>(const_cast<std::string*>(&exepath)),
                                       reinterpret_cast<void*>(&bEven));
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
    }
    catch(...)
    {
    }

    cerr<<"\r"<<std::left<<std::setw(40)<<' '<<endl;

    db.close();

    if(TaskController::GetController().ExistErrors())
    {
        std::cout<<"\n==============Hydrus has finished "
                <<TaskController::GetController().TaskCount()<<" cases with errors.==============\n";
        if(TaskController::GetController().LogingErrorInDB())
        {
            std::cout << "!!!!!!!!!! There are some errors during the runing. See table errlog for more details !!!!!!!!!!\n\n";
        }
        else if(TaskController::GetController().LogingErrorInFile())
        {
            std::cout << "!!!!!!!!!! There are some errors during the runing. See file"
                      <<TaskController::GetController().LogFile() << " for more details !!!!!!!!!!\n\n";
        }
    }
    else
    {
        std::cout<<"\n==============Hydrus has successfully finished. "
                <<TaskController::GetController().TaskCount()<<" cases have been done.==============\n\n";
    }
    return 0;
}
