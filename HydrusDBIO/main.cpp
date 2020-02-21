
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
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <string>
#include <tclap/CmdLine.h>
#include <vector>
#include <QDir>
#include <memory>
#include <future>
#include "HydrusParameterFilesManager.h"

using namespace std;
using namespace TCLAP;

bool _bFinished;

bool ExecuteOperation(const int op,const int gid,void* ppath,void* pqry)
{
    std::string& path=*static_cast<std::string*>(ppath);
    QSqlQuery& qry=*static_cast<QSqlQuery*>(pqry);
    HydrusParameterFilesManager h(gid,path,qry);
    bool result;
    switch (op)
    {
    case 0:
        result=h.ImportInputFiles();
        break;
    case 1:
        result=h.ExportResultFiles();
        break;
    case 2:
        result=h.DropResultFiles();
        break;
    default:
        result=h.DropCase();
        break;
    }
    _bFinished=true;
    return result;
}

void Execute(const int op,const int gid,const std::string& path,QSqlQuery& qry)
{
    _bFinished=false;
    bool result=false;
    std::string message="Operation is exectuing, please wait  ";
    std::cout<<message;
    std::string failmessage[]={"Import failed!","Export failed!","Drop failed!","Drop failed!"};
    std::string successmessage[]={"Import successfully!","Export successfully!","Drop successfully!","Drop successfully!"};
    try
    {
        std::future<bool> v=std::async(std::launch::async,ExecuteOperation,op,gid,
                                       reinterpret_cast<void*>(const_cast<std::string*>(&path)),reinterpret_cast<void*>(&qry));
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
        cerr<<"\r"<<std::left<<std::setw(40)<<failmessage[op]<<endl;
    }
    else
    {
        cout<<"\r"<<std::left<<std::setw(40)<<successmessage[op]<<endl;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication _coreapp(argc,argv);
    
    CmdLine cmd("This program is used to import/export  hydrus input or output files into/from database.");

    ValueArg<string> dbname("D","dbname","the database name",true,"","string");
    ValueArg<string> user("U","username","the username",true,"","string");
    ValueArg<string> password("P","password","the password",true,"","string");
    ValueArg<std::string> host("H","hostname","the host name",false,"localhost","string");

    SwitchArg argimport("i","import","import the hydrus input files into database");
    SwitchArg argexport("e","export","export the hydrus output files from database");
    SwitchArg argdorpHydruscase("","dropcase","remove input paramters and results from database");
    SwitchArg argdropResult("","dropresults","remove results from database");

    vector<Arg*>  xorlist;
    xorlist.push_back(&argimport);
    xorlist.push_back(&argexport);
    xorlist.push_back(&argdorpHydruscase);
    xorlist.push_back(&argdropResult);

    ValueArg<int> gid("g","gid","the id as imported id or id for export from database",true,0,"integer");

    ValueArg<string> filepath("p","path","the path to the hydrus input files or path to store exported hydrus result files",false,"","string");

    //ValueArg<string> logfile("","logerr","the logging error filename",false,"","string");

    cmd.add(dbname);
    cmd.add(user);
    cmd.add(password);
    cmd.add(host);

    cmd.xorAdd(xorlist);

    cmd.add(filepath);
    cmd.add(gid);

    //cmd.add(logfile);

    cmd.parse(argc,argv);

    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL","IODB");
    db.setHostName(host.getValue().c_str());
    db.setUserName(user.getValue().c_str());
    db.setDatabaseName(dbname.getValue().c_str());
    db.setPassword(password.getValue().c_str());
    if(!db.open())
    {
        cout<<"Can't connect database!"<<endl;
        return 0;
    }
    std::shared_ptr<QSqlQuery> qry(new QSqlQuery(db));


    if(filepath.isSet() && argimport.getValue())
    {
        QDir p(filepath.getValue().c_str());
        std::string abpath = QDir::toNativeSeparators(p.absolutePath()).toStdString();
        if(!p.exists())
        {
            cerr<<"Not Valid Path:\t"<<abpath<<endl;
            return 0;
        }
        Execute(0,gid.getValue(),abpath,*qry);
    }
    else if(filepath.isSet() &&  argexport.getValue())
    {
        QDir p(filepath.getValue().c_str());
        std::string abpath = QDir::toNativeSeparators(p.absolutePath()).toStdString();
        if(!p.exists())
        {
            if( !p.mkpath(abpath.c_str()))
            {
                cerr<<"Can not create Path"<<abpath<<endl;
                return 0;
            }

        }
        Execute(1,gid.getValue(),abpath,*qry);
    }
    else if(argdropResult.getValue())
    {
        Execute(2,gid.getValue(),"",*qry);
    }
    else if(argdorpHydruscase.getValue())
    {
        Execute(3,gid.getValue(),"",*qry);
    }
    else
    {
        cerr<<"Not Valid Parameters. "
              "-i or -e need the company of path"
           <<endl;
    }
    qry->finish();
    db.close();
    return 0;
}
