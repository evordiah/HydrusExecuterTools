
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
#include <QSqlQuery>
#include <QVariant>
#include <string>
#include <tclap/CmdLine.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <queue>
#include <future>
#include "HydrusExcuter.h"
#include "hydrusexcuterevenly.h"
#include "hydrusexcuterpre.h"
#include "importhydrusoutputfile.h"
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <unistd.h>


std::queue<std::string> _gbuftoinserdb;
int _gCounter;
std::string _glogFile;
bool _gErrInDb;
bool _gErrInFile;

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

int GetAvailableCpuCount()
{
        return sysconf(_SC_NPROCESSORS_ONLN);
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
            HydrusExcuter::gids.push(qry.value(0).toInt());
        }
    }
    else
    {
        std::cout<<"Can't get valid ids from database! May be you give invalid where criteria!"<<std::endl;
        return false;
    }
    return true;
}

bool GetIdsFromList(QSqlDatabase& db,std::string& idlist)
{
    while(!idlist.empty() && idlist.back()==' ')
    {
        idlist.pop_back();
    }
    if(idlist.empty())
    {
        return false;
    }
    std::stringstream strbld(idlist);
	std::stringstream strbldsql;
	strbldsql<<"select gid from selector where status='todo' and gid in (";
    std::string sv;
    while(std::getline(strbld,sv,' '))
    {
        if(sv.empty())
        {
            continue;
        }
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
			HydrusExcuter::gids.push(qry.value(0).toInt());
		}
	}
	else
	{
		return false;
	}
    return true;
}

bool GetIdsFromFile(QSqlDatabase& db, std::string& filename)
{
    if(!QFileInfo::exists(filename.c_str()))
    {
        std::cout<<filename<<" is not a valid file!"<<std::endl;
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
			HydrusExcuter::gids.push(qry.value(0).toInt());
		}
	}
	else
	{
		return false;
	}
    return true;
}

bool ParseCmdVars(int argc, char *argv[],std::string& strhydrusexefile,
                  std::string& strexepath,int& threadcnt,QSqlDatabase& db,bool& bEven)
{
    using namespace TCLAP;

    CmdLine cmd("This program is used to execute hydrus and import results into database.");

    ValueArg<std::string> dbname("d","databasename","the database name",true,"","string");
    ValueArg<std::string> user("u","username","the username",true,"","string");
    ValueArg<std::string> password("p","password","the password",true,"","string");
    ValueArg<std::string> host("","hostname","the host name",false,"localhost","string");
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

    ValueArg<std::string> dest("","exepath","the folder for saving temp files and execute hydrus",false,"/tmp/executing","string");
    cmd.add(dest);

    ValueArg<std::string> logarg("","log","the file for recording errors ",false,"/tmp/hydrusexecutingerrlog.log","string");
    cmd.add(logarg);

    ValueArg<int> threadcntarg("j","thread_count","set how many threads to run parally",false,GetAvailableCpuCount(),"integer");
    cmd.add(threadcntarg);

    ValueArg<std::string> hydrusexearg("e","hydrus","set executable hydrus file to run",true,"","string");
    cmd.add(hydrusexearg);

    SwitchArg exestrategy("","evenly","Set a parallel execution policy, set this parameter, resulting in the average assignment of calculated tasks",false);
    cmd.add(exestrategy);


    cmd.parse(argc,argv);

    if(!InitialDB(host.getValue(),dbname.getValue(),user.getValue(),password.getValue(),db))
    {
        return false;
    }

    if(exestrategy.isSet())
    {
        bEven=true;
    }
    else
    {
        bEven=false;
    }

    threadcnt=threadcntarg.getValue();
    if(threadcnt<=0)
    {
        threadcnt=GetAvailableCpuCount();
    }

    strexepath=dest.getValue();
    QDir p(strexepath.c_str());

    if(!p.exists())
    {
        if(!p.mkpath(strexepath.c_str()))
        {
            std::cout<<"Can not find or create path "<<strexepath<<std::endl;
            return false;
        }
    }

    strhydrusexefile=hydrusexearg.getValue();
    if(!QFileInfo::exists(strhydrusexefile.c_str()))
    {
        std::cout<<"Can not find "<<strhydrusexefile<<std::endl;
        return false;
    }

    _glogFile=logarg.getValue();

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

bool DbLogError(int gid,const std::string& error,std::shared_ptr<QSqlQuery> qry)
{
    std::stringstream strbld;
    strbld<<"insert into errlog(gid, errordes) values("<<gid<<",'"<<error<<"');";
	_gErrInDb = true;
    return qry->exec(strbld.str().c_str());
}

void LogError(const std::string &err)
{
	_gErrInFile = true;
    std::ofstream out(_glogFile,std::ios_base::app);
    out<<err<<std::endl;
    out.close();
}

//filename must be a binary file created by HydrusResultCompresser.
void InsertintoDB(const std::string& filename,std::shared_ptr<QSqlQuery> qry)
{
    QFileInfo qfinf(filename.c_str());
    try
    {
        std::string strgid=qfinf.baseName().toStdString();
        ImportHydrusoutputFile imp;
        int gid=stoi(strgid);
        imp.Gid(gid);
        imp.Filename(filename);
        if(!imp)
        {
            if(!DbLogError(gid,"do not converge",qry))
            {
                std::stringstream strbld;
                strbld<<gid<<" is not a converged result!"<<std::endl;
                LogError(strbld.str());
            }
        }
        else if(!imp.Execute(qry))
        {
            if(!DbLogError(gid,"import failed!",qry))
            {
                std::stringstream strbld;
                strbld<<gid<<" import failed!"<<std::endl;
                LogError(strbld.str());
            }
        }

       QDir pf(filename.c_str());
       pf.remove(filename.c_str());
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

void DealResult(std::shared_ptr<QSqlQuery> qry)
{
    while(_gCounter)
    {
        std::this_thread::sleep_for(std::chrono::seconds(20));
        {
            std::unique_lock<std::mutex> ul(HydrusExcuter::bufResMutex);
            HydrusExcuter::bufCondVar.wait(ul,[] {return !HydrusExcuter::bufresultsqueue.empty();});
            while(!HydrusExcuter::bufresultsqueue.empty())
            {
                _gbuftoinserdb.push(HydrusExcuter::bufresultsqueue.front());
                HydrusExcuter::bufresultsqueue.pop();
            }
        }
        while(!_gbuftoinserdb.empty())
        {
            std::string v=_gbuftoinserdb.front();
            _gbuftoinserdb.pop();
            if(v!="error")
            {
                InsertintoDB(v,qry);
            }
            _gCounter--;
        }
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

std::string GetValidConName()
{
    return QUuid::createUuid().toString().toStdString();
}

void GetTaskList(std::vector<int>& vecgid,int threadcnt,int threadindex)
{
    vecgid.clear();
    int cnt=_gCounter/threadcnt;
    int res=_gCounter-threadcnt*cnt;
    for(int i=0;i<cnt;++i)
    {
       vecgid.push_back(HydrusExcuter::gids.front());
       HydrusExcuter::gids.pop();
    }
    if(threadindex<res)
    {
        vecgid.push_back(HydrusExcuter::gids.front());
        HydrusExcuter::gids.pop();
    }
}

int main(int argc, char *argv[])
{
    using namespace std;
    QCoreApplication _coreApp(argc,argv);
    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL",GetValidConName().c_str());
    string exepath,hydrusexefile;
    int igiventhreadcnt;
    bool bEven;
    if(!ParseCmdVars(argc,argv,hydrusexefile,exepath,igiventhreadcnt,db,bEven))
    {
        std::cout<<"\n*********The parameters given is not valid. Exit now.*********\n\n";
        return 0;
    }

    _gCounter=HydrusExcuter::gids.size();
    if(_gCounter==0)
    {
        std::cout<<"\n*********There no valid gids. Exit now.*********\n\n";
        return 0;
    }

    int threadcnt=_gCounter< igiventhreadcnt ? _gCounter : igiventhreadcnt;

    if(!ProviderHydrusExe(hydrusexefile,exepath))
    {
        std::cout<<"\n*********The hydrus executable file is not OK. Exit now.*********\n\n";
        return 0;
    }

    std::vector<std::shared_ptr<HydrusExcuter>> _executers;
    std::vector<std::future<void>> _futures;
    std::vector<int> vecgid;
    std::shared_ptr<HydrusExcuter> pHydrusExe;
    for(int i=0;i<threadcnt;++i)
    {
        if(bEven)
        {
            GetTaskList(vecgid,threadcnt,i);
            pHydrusExe.reset(new HydrusExcuterEvenly(exepath,vecgid));
        }
        else
        {
            pHydrusExe.reset(new HydrusExcuterPre(exepath));
        }
        pHydrusExe->DataBase(db);
        _executers.push_back(pHydrusExe);
        _futures.push_back(std::async(std::launch::async, &HydrusExcuter::Execute,_executers[i]));
    }
    std::shared_ptr<QSqlQuery> qry(new QSqlQuery(db));
    int gidcnt=_gCounter;
    DealResult(qry);
    qry->finish();
    db.close();
    _executers.clear();
    _futures.clear();
    QDir qexepath(exepath.c_str());
    qexepath.removeRecursively();
    std::cout<<"\n==============Hydrus has successfully finished. "
             <<gidcnt<<" cases have been done.==============\n\n";
	if (_gErrInDb && _gErrInFile)
	{
		std::cout << "!!!!!!!!!! There are some errors during the runing. See table errlog and file "
			      << _glogFile << " for more details !!!!!!!!!!\n\n";
	}
	else if (_gErrInDb)
	{
		std::cout << "!!!!!!!!!! There are some errors during the runing. See table errlog for more details !!!!!!!!!!\n\n";
	}
	else if(_gErrInFile)
	{
		std::cout << "!!!!!!!!!! There are some errors during the runing. See file"
			      <<_glogFile<<" for more details !!!!!!!!!!\n\n";
	}
    return 0;
}
