
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

#include "exporter.h"
#include <regex>
#include <iostream>
#include <string>
#include "exporthydrusinputfile.h"
#include "exporthydrusoutputfile.h"
#include "HydrusInputCompresser.h"
#include "HydrusResultCompresser.h"
#include <QSqlQuery>
#include <sstream>
#include <fstream>
#include <QDir>
#include <QFileInfo>

std::string Exporter::_logFile="";

bool Exporter::exportInputFiles(std::vector<int>& gids, const std::string& dest, std::shared_ptr<QSqlQuery> qry, bool bAscII)
{
    bool result=true;

	QDir p(dest.c_str());
	if (!p.exists())
	{
		p.mkpath(dest.c_str());
	}
    std::stringstream strbld;
    for(auto it=gids.begin();it!=gids.end();++it)
    {
        int gid=*it;
        if(bAscII)
        {
            strbld.clear();
            strbld.str("");
            strbld<<gid;
			std::string pth = QDir::toNativeSeparators(p.absoluteFilePath(strbld.str().c_str())).toStdString();
			QDir pd(pth.c_str());
			if (!pd.exists())
			{
				pd.mkpath(pth.c_str());
			}
			result =  exportInputFile(gid,pth,qry,true) && result;
        }
        else
        {
            result =  exportInputFile(gid,dest,qry,false) && result;
        }
    }
    return result;
}

bool Exporter::exportOutputFiles(std::vector<int>& gids, const std::string& dest, std::shared_ptr<QSqlQuery> qry, bool bAscII)
{
    bool result=true;

	QDir p(dest.c_str());
	if (!p.exists())
	{
		p.mkpath(dest.c_str());
	}
    std::stringstream strbld;
    for(auto it=gids.begin();it!=gids.end();++it)
    {
        int gid=*it;
        if(bAscII)
        {
            strbld.clear();
            strbld.str("");
            strbld<<gid;
			std::string pth = QDir::toNativeSeparators(p.absoluteFilePath(strbld.str().c_str())).toStdString();
			QDir pd(pth.c_str());
			if (!pd.exists())
			{
				pd.mkpath(pth.c_str());
			}
            result =  exportOutputFile(gid,pth,qry,true) && result;
        }
        else
        {
            result =  exportOutputFile(gid,dest,qry,false) && result;
        }
    }
    return result;
}

bool Exporter::exportOutputFile(int gid, const std::string &dest, std::shared_ptr<QSqlQuery> qry, bool bAscII)
{
    bool result=true;

	QDir p(dest.c_str());
	if (!p.exists())
	{
		p.mkpath(dest.c_str());
	}
    std::string filename;
    if(bAscII)
    {
        filename=dest;
    }
    else
    {
        std::stringstream strbld;
        strbld<<gid<<".bin";
		filename = QDir::toNativeSeparators(p.absoluteFilePath(strbld.str().c_str())).toStdString();
    }

    ExportHydrusOutputFile exp(qry);
    exp.Gid(gid);
    if(!exp.Execute(filename))
    {
        if(!Exporter::_logFile.empty())
        {
            std::stringstream strbld;
            strbld<<gid<<" export failed!"<<std::endl;
            LogError(strbld.str());
        }
        result=false;
    }
    return result;
}

void Exporter::LogError(const std::string &err)
{
    std::ofstream out(_logFile,std::ios_base::app);
    out<<err<<std::endl;
    out.close();
}

bool Exporter::exportInputFile(int gid, const std::string &dest, std::shared_ptr<QSqlQuery> qry, bool bAscII)
{
    bool result=true;

	QDir p(dest.c_str());
	if (!p.exists())
	{
		p.mkpath(dest.c_str());
	}
    std::string filename;
    if(bAscII)
    {
        filename=dest;
    }
    else
    {
        std::stringstream strbld;
        strbld<<gid<<".bin";
		filename = QDir::toNativeSeparators(p.absoluteFilePath(strbld.str().c_str())).toStdString();
    }

    ExportHydrusInputFile exp(qry);
    exp.Gid(gid);
    if(!exp.Execute(filename))
    {
        if(!Exporter::_logFile.empty())
        {
            std::stringstream strbld;
            strbld<<gid<<" export failed!"<<std::endl;
            LogError(strbld.str());
        }
        result=false;
    }
    return result;

}
