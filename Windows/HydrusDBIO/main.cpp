
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
#include <string>
#include <tclap/CmdLine.h>
#include <vector>
#include <QDir>
#include "importer.h"
#include "exporter.h"
#include <memory>

using namespace std;
using namespace TCLAP;

bool ImportData(int gid,const std::string& path,std::shared_ptr<QSqlQuery> qry)
{
    return Importer::importInputFile(gid,path,qry);
}

bool ExportData(int gid,const string& path,std::shared_ptr<QSqlQuery> qry)
{
	QDir destpath(path.c_str());
	if (!destpath.exists())
	{
		destpath.mkpath(path.c_str());
	}
    return Exporter::exportOutputFile(gid,path,qry);
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc,argv);
    CmdLine cmd("This program is used to import/export  hydrus input or output files into/from database.");

    ValueArg<string> dbname("D","dbname","the database name",true,"","string");
    ValueArg<string> user("U","username","the username",true,"","string");
    ValueArg<string> password("P","password","the password",true,"","string");
    ValueArg<std::string> host("H","hostname","the host name",false,"localhost","string");

    SwitchArg argimport("i","import","import the hydrus input files into database",true);
    SwitchArg argexport("e","export","export the hydrus output files from database",true);

    ValueArg<string> filepath("p","path","the path to the hydrus input files or path to store exported hydrus result files",true,"","string");
    ValueArg<int> gid("g","gid","the id as imported id or id for export from database",true,0,"integer");

    ValueArg<string> logfile("","logerr","the logging error filename",false,"","string");

    cmd.add(dbname);
    cmd.add(user);
    cmd.add(password);
    cmd.add(host);

    cmd.xorAdd(argimport,argexport);

    cmd.add(filepath);
    cmd.add(gid);

    cmd.add(logfile);

    cmd.parse(argc,argv);

    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL","IEDB");
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

    string logfilename="";

    if(logfile.isSet())
    {
		QFileInfo p(logfile.getValue().c_str());
		QDir ppath = p.dir().absolutePath();
		if (!ppath.exists())
		{
			ppath.mkpath(ppath.absolutePath());
		}
		logfilename = logfile.getValue();
    }

	QDir p(filepath.getValue().c_str());
	std::string abpath = QDir::toNativeSeparators(p.absolutePath()).toStdString();
    if(argimport.isSet())
    {
        if(!logfilename.empty())
        {
            Importer::SetLogFile(logfilename);
        }
        ImportData(gid.getValue(),abpath,qry);
    }
    else
    {
        if(!logfilename.empty())
        {
            Exporter::SetLogFile(logfilename);
        }
        ExportData(gid.getValue(),abpath,qry);
    }
    qry->finish();
    db.close();
    return 0;
}
