
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

#include "HydrusResultCompresser.h"
#include "A_LevelObject.h"
#include "T_LevelObject.h"
#include "BalanceObject.h"
#include "NodeInfoObject.h"
#include "RuninfoParser.h"
#include "obs_nodeobject.h"
#include <QDir>
#include <QFileInfo>
#include <fstream>

bool HydrusResultCompresser::Compress(const std::string& sourcepath,const std::string& tofilename,bool RemoveSrcFiles)
{
    bool result=true;

    using namespace std;
	QDir spath(sourcepath.c_str());
	if(!spath.exists())
    {
        return false;
    }

    int offset[7]={0,5,0,0,0,0,0};

    QString files[]= {"A_Level.out","T_Level.out","Nod_Inf.out","Balance.out","Run_Inf.out","Obs_Node.out"};
	std::string path[6];
	path[0] = spath.absoluteFilePath(files[0]).toStdString();
	bool bUppercase = !QFileInfo::exists(path[0].c_str());
    for(int i=0; i<6; ++i)
    {
        if(bUppercase)
        {
			path[i] = QDir::toNativeSeparators(spath.absoluteFilePath(files[i].toUpper())).toStdString();
        }
        else
        {
			path[i] = QDir::toNativeSeparators(spath.absoluteFilePath(files[i])).toStdString();
        }
        if(!QFileInfo::exists(path[i].c_str()))
        {
            if(i<5)
            {
                return false;
            }
            else
            {
                offset[1]=4;
            }
        }
    }
    try
    {
        int startoffset=sizeof(int)*7;
        RuninfoEncoder runinfo(path[4]);
        offset[0]=runinfo.NotConvergency();
        A_LevelEncoder alevel(path[0]);
        offset[2]=startoffset;
        T_LevelEncoder tlevel(path[1]);
        offset[3]=offset[2]+alevel.GetLength();
        Nod_InfEncoder nodinfo(path[2]);
        offset[4]=offset[3]+tlevel.GetLength();
        BalanceEncoder balance(path[3]);
        offset[5]=offset[4]+nodinfo.GetLength();
        offset[6]=offset[5]+balance.GetLength();
		QFileInfo tpath(tofilename.c_str());
		QDir ptpath = tpath.dir();
		if (!ptpath.exists())
		{
			ptpath.mkpath(ptpath.absolutePath());
		}
        ofstream out(tofilename,ios_base::binary);
        out.write((const char*)offset,startoffset);
        out<<alevel<<tlevel<<nodinfo<<balance;
        if(offset[1]==5)
        {
            Obs_NodeObject obsnode(path[5]);
            out<<obsnode;
        }
        out.close();
    }
    catch(...)
    {
        result=false;
    }
    if(RemoveSrcFiles)
    {
		spath.removeRecursively();
    }
    return result;
}

bool HydrusResultCompresser::Compress(const A_LevelObject &aobj, const T_LevelObject &tobj, const NodeInfoObject &nobj, const BalanceObject &bobj, const std::string &tofilename)
{
    int offset[7]={0,4,0,0,0,0,0};
    int startoffset=sizeof(int)*7;
    offset[2]=startoffset;
    offset[3]=offset[2]+aobj.GetLength();
    offset[4]=offset[3]+tobj.GetLength();
    offset[5]=offset[4]+nobj.GetLength();
	QFileInfo tpath(tofilename.c_str());
	QDir ptpath = tpath.dir();
	if (!ptpath.exists())
	{
		ptpath.mkpath(ptpath.absolutePath());
	}
    std::ofstream out(tofilename,std::ios_base::binary);
    out.write((const char*)offset,startoffset);
    out<<aobj<<tobj<<nobj<<bobj;
    out.close();
    return true;
}

bool HydrusResultCompresser::Compress(const A_LevelObject &aobj, const T_LevelObject &tobj, const NodeInfoObject &nobj, const BalanceObject &bobj, const Obs_NodeObject &oobj, const std::string &tofilename)
{
    int offset[7]={0,5,0,0,0,0,0};
    int startoffset=sizeof(int)*7;
    offset[2]=startoffset;
    offset[3]=offset[2]+aobj.GetLength();
    offset[4]=offset[3]+tobj.GetLength();
    offset[5]=offset[4]+nobj.GetLength();
    offset[6]=offset[5]+bobj.GetLength();
	QFileInfo tpath(tofilename.c_str());
	QDir ptpath = tpath.dir();
	if (!ptpath.exists())
	{
		ptpath.mkpath(ptpath.absolutePath());
	}
    std::ofstream out(tofilename,std::ios_base::binary);
    out.write((const char*)offset,startoffset);
    out<<aobj<<tobj<<nobj<<bobj<<oobj;
    out.close();
    return true;
}


bool HydrusResultCompresser::IsConverged(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return false;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int v;
    in.read((char*)&v,sizeof(int));
    in.close();
    return v==0?true:false;
}

int HydrusResultCompresser::GetPartCount(const std::string &filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return 0;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int v[2];
    in.read((char*)v,sizeof(int)*2);
    in.close();
    return v[1];
}

bool HydrusResultCompresser::UnCompress(const std::string &filename, const std::string &topath)
{
    using namespace std;
	QDir despath(topath.c_str());
	if (!despath.exists())
	{
		despath.mkpath(topath.c_str());
	}
    std::string files[]= {"A_Level.out","T_Level.out","Nod_Inf.out","Balance.out","Obs_Node.out"};
	std::string path[5];
    for(int i=0; i<5; ++i)
    {
        path[i]=QDir::toNativeSeparators(despath.absoluteFilePath(files[i].c_str())).toStdString();
    }
	if(!QFileInfo::exists(filename.c_str()))
    {
        std::cout<<filename<<" does not exist!"<<std::endl;
        return false;
    }
    ifstream in(filename,ios_base::binary);
    int offset[7];
    in.read((char*)offset,sizeof(int)*7);
    A_LevelObject alevel(in);
    ofstream out(path[0]);
    out<<alevel;
    out.close();
    T_LevelObject tlevel(in);
    out.open(path[1]);
    out<<tlevel;
    out.close();
    NodeInfoObject nod(in);
    out.open(path[2]);
    out<<nod;
    out.close();
    BalanceObject bal(in);
    out.open(path[3]);
    out<<bal;
    out.close();
    if(offset[1]==5)
    {
        Obs_NodeObject obs(in);
        out.open(path[4]);
        out<<obs;
        out.close();
    }
    in.close();
    return true;
}

std::unique_ptr<A_LevelObject> HydrusResultCompresser::ExtractALevel(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[7];
    in.read((char*)offset,sizeof(int)*7);
    std::unique_ptr<A_LevelObject> result(new A_LevelObject(in));
    in.close();
    return result;
}

std::unique_ptr<T_LevelObject> HydrusResultCompresser::ExtractTLevel(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[7];
    in.read((char*)offset,sizeof(int)*7);
    in.seekg(offset[3],std::ios_base::beg);
    std::unique_ptr<T_LevelObject> result(new T_LevelObject(in));
    in.close();
    return result;
}

std::unique_ptr<NodeInfoObject> HydrusResultCompresser::ExtractNodeInfo(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[7];
    in.read((char*)offset,sizeof(int)*7);
    in.seekg(offset[4],std::ios_base::beg);
    std::unique_ptr<NodeInfoObject> result (new NodeInfoObject(in));
    in.close();
    return result;
}

std::unique_ptr<BalanceObject> HydrusResultCompresser::ExtractBalance(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[7];
    in.read((char*)offset,sizeof(int)*7);
    in.seekg(offset[5],std::ios_base::beg);
    std::unique_ptr<BalanceObject> result (new BalanceObject(in));
    in.close();
    return result;
}

std::unique_ptr<Obs_NodeObject> HydrusResultCompresser::ExtractObsNode(const std::string &filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::unique_ptr<Obs_NodeObject> result;
    std::ifstream in(filename,std::ios_base::binary);
    int offset[7];
    in.read((char*)offset,sizeof(int)*7);
    if(offset[1]==5)
    {
        in.seekg(offset[6],std::ios_base::beg);
        result.reset(new Obs_NodeObject(in));
    }
    in.close();
    return result;
}
