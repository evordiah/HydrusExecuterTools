
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

#include "importer.h"
#include <QFileInfo>
#include <QDir>
#include <regex>
#include <iostream>
#include <string>
#include "importhydrusinputfile.h"
#include "importhydrusoutputfile.h"
#include "HydrusInputCompresser.h"
#include "HydrusResultCompresser.h"
#include <QSqlQuery>
#include <sstream>
#include <fstream>

void Importer::importInputFiles(std::vector<std::string> &filelist, std::shared_ptr<QSqlQuery> qry)
{
    std::regex reg("\\d+\\.bin");
    for(auto it=filelist.begin();it!=filelist.end();++it)
    {
		std::string file = *it;
		QFileInfo qinf(file.c_str());
		if(!qinf.exists())
        {
            continue;
        }
		std::string fn = qinf.fileName().toStdString();
        if(!std::regex_match(fn,reg))
        {
            std::cout<<file<<" is not a valid hydrusinputfile!"<<std::endl;
            continue;
        }
		std::string strgid = qinf.baseName().toStdString();
        ImportHydrusinputFile imp;
        imp.Gid(stoi(strgid));
        imp.Filename(file);
        if(!imp)
        {
            if(!Importer::_logFile.empty())
            {
                std::stringstream strbld;
                strbld<<file<<" happen unknow error!"<<std::endl;
                LogError(strbld.str());
            }
        }
        if(!imp.Execute(qry))
        {
            if(!Importer::_logFile.empty())
            {
                std::stringstream strbld;
                strbld<<file<<" import failed!"<<std::endl;
                LogError(strbld.str());
            }
        }
    }
}

std::string Importer::_logFile="";

void Importer::importOutputFiles(std::vector<std::string> &filelist, std::shared_ptr<QSqlQuery> qry)
{
    std::regex reg("\\d+\\.bin");
    for(auto it=filelist.begin();it!=filelist.end();++it)
    {
		std::string file = *it;
		QFileInfo qinf(file.c_str());
		if(!qinf.exists())
        {
            continue;
        }
		std::string fn = qinf.fileName().toStdString();
        if(!std::regex_match(fn,reg))
        {
            std::cout<<file<<" is not a valid hydrusoutputfile!"<<std::endl;
            continue;
        }
        std::string strgid=qinf.baseName().toStdString();
        ImportHydrusoutputFile imp;
        imp.Gid(stoi(strgid));
        imp.Filename(file);
        if(!imp)
        {
            if(!Importer::_logFile.empty())
            {
                std::stringstream strbld;
                strbld<<file<<" is not a converged result!"<<std::endl;
                LogError(strbld.str());
            }
        }
        if(!imp.Execute(qry))
        {
            if(!Importer::_logFile.empty())
            {
                std::stringstream strbld;
                strbld<<file<<" import failed!"<<std::endl;
                LogError(strbld.str());
            }
        }
    }
}

bool Importer::importInputFile(int gid, const std::string &path, std::shared_ptr<QSqlQuery> qry)
{
    bool result=true;

    std::stringstream strbld;
    strbld<<gid<<".bin";
	QDir p(path.c_str());
	std::string filename = QDir::toNativeSeparators(p.absoluteFilePath(strbld.str().c_str())).toStdString();
    if(!HydrusInputCompresser::Compress(path,filename))
    {
       return false;
    }
    ImportHydrusinputFile imp;
    imp.Gid(gid);
    imp.Filename(filename);
    if(!imp)
    {
        if(!Importer::_logFile.empty())
        {
            std::stringstream strbld;
            strbld<<path<<" happen unknow error!"<<std::endl;
            LogError(strbld.str());
        }
        result=false;
    }
    else if(!imp.Execute(qry))
    {
        if(!Importer::_logFile.empty())
        {
            std::stringstream strbld;
            strbld<<path<<" import failed!"<<std::endl;
            LogError(strbld.str());
        }
        result=false;
    }
	p.remove(filename.c_str());
    return result;
}

bool Importer::importOutputFile(int gid, const std::string &path, std::shared_ptr<QSqlQuery> qry)
{
    bool result=true;

    std::stringstream strbld;
    strbld<<gid<<".bin";
	QDir p(path.c_str());
	std::string filename = QDir::toNativeSeparators(p.absoluteFilePath(strbld.str().c_str())).toStdString();
    if(!HydrusResultCompresser::Compress(path,filename,false))
    {
       return false;
    }
    ImportHydrusoutputFile imp;
    imp.Gid(gid);
    imp.Filename(filename);
    if(!imp)
    {
        if(!Importer::_logFile.empty())
        {
            std::stringstream strbld;
            strbld<<path<<" happen unknow error!"<<std::endl;
            LogError(strbld.str());
        }
        result=false;
    }
    else if(!imp.Execute(qry))
    {
        if(!Importer::_logFile.empty())
        {
            std::stringstream strbld;
            strbld<<path<<" import failed!"<<std::endl;
            LogError(strbld.str());
        }
        result=false;
    }
	p.remove(filename.c_str());
    return result;
}

void Importer::LogError(const std::string &err)
{
    std::ofstream out(_logFile,std::ios_base::app);
    out<<err<<std::endl;
    out.close();
}

