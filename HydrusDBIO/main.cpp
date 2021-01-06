
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

#include <pqxx/pqxx>
#include <regex>
#include <string>
#include <tclap/CmdLine.h>
#include <vector>
#include <filesystem>
#include <memory>
#include <future>
#include "Stringhelper.h"
#include "HydrusParameterFilesManager.h"

using namespace std;
using namespace TCLAP;

bool _bFinished;

bool ExecuteOperation(const int op,const int gid,void* ppath,void* pqry)
{

    std::string& path=*static_cast<std::string*>(ppath);
    pqxx::connection &qry=*static_cast<pqxx::connection *>(pqry);
    HydrusParameterFilesManager h(gid,path,qry);
    //HydrusParameterFilesManager h(gid,path,qry,3,2,0);
    bool result;
    switch (op)
    {
    case 0:
        result=h.ImportInputFiles();
        //result=h.ImportResultFiles();
        break;
    case 1:
        result=h.ExportResultFiles();
        //result=h.ExportInputFiles();
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

void Execute(const int op,const int gid,const std::string& path,pqxx::connection * qry)
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
                                       reinterpret_cast<void*>(const_cast<std::string*>(&path)),reinterpret_cast<void*>(qry));
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

    CmdLine cmd("This program is used to import/export  hydrus input or output files into/from database.");

    ValueArg<string> dbname("D","dbname","the database name",true,"","string");
    ValueArg<string> user("U","username","the username",true,"","string");
    ValueArg<string> password("P","password","the password",true,"","string");
    ValueArg<std::string> host("H","hostname","the host name, for example [127.0.0.1][:5432] or [localhost][:5432]",false,"localhost:5432","string");

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
    string hostname=host.getValue();
    string port;
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
                return 0;
            }
        }
    }
    std::string connstr="host=";
    connstr.append(hostname);
    connstr.append(" dbname=");
    connstr.append(dbname.getValue());
    connstr.append(" user=");
    connstr.append(user.getValue());
    connstr.append(" password=");
    connstr.append(password.getValue());
    connstr.append(" port=");
    connstr.append(port);
    std::unique_ptr<pqxx::connection> qry;
    try
    {
        qry=std::make_unique<pqxx::connection>(connstr);
    }
    catch (...)
    {
        cout<<"Can't connect database!"<<endl;
        return 0;
    }

    if(filepath.isSet() && argimport.getValue())
    {
        std::filesystem::path p=filepath.getValue();
        std::filesystem::path abpath = std::filesystem::absolute(p);
        //std::string spath=abpath.native().c_str();
		std::string spath = abpath.string();
        if(!std::filesystem::exists(abpath))
        {
            cerr<<"Not Valid Path:\t"<<spath<<endl;
            return 0;
        }
        Execute(0,gid.getValue(),spath,qry.get());
    }
    else if(filepath.isSet() &&  argexport.getValue())
    {
        std::filesystem::path p=filepath.getValue();
        std::filesystem::path abpath = std::filesystem::absolute(p);
        //std::string spath=abpath.native().c_str();
		std::string spath = abpath.string();
        if(!std::filesystem::exists(abpath))
        {
            if( !std::filesystem::create_directories(abpath))
            {
                cerr<<"Can not create Path"<<spath<<endl;
                return 0;
            }

        }
        Execute(1,gid.getValue(),spath,qry.get());
    }
    else if(argdropResult.getValue())
    {
        Execute(2,gid.getValue(),"",qry.get());
    }
    else if(argdorpHydruscase.getValue())
    {
        Execute(3,gid.getValue(),"",qry.get());
    }
    else
    {
        cerr<<"Not Valid Parameters. "
              "-i or -e need the company of path"
           <<endl;
    }
    return 0;
}
