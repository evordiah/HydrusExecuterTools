
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

#include "HydrusInputCompresser.h"
#include <QFileInfo>
#include <QDir>
#include "selectorobject.h"
#include "atmosphobject.h"
#include "profileobject.h"
#include <fstream>

bool HydrusInputCompresser::Compress(const std::string& srcpath,const std::string& tofilename)
{
	QFileInfo p(srcpath.c_str());
    if(!p.exists())
    {
        std::cout<<"Path "<<srcpath<<" does not exist!"<<std::endl;
        return false;
    }
    if(!p.isDir())
    {
        std::cout<<srcpath<<" must be a folder"<<std::endl;
        return false;
    }
    std::string selectorfilename="SELECTOR.IN";
    std::string profilename="PROFILE.DAT";
    std::string atmosphfilename="ATMOSPH.IN";
    SelectorObject sobj;
    ProfileObject pobj;
    AtmosphObject atobj;
    int count=0;
	QDir path(p.absoluteFilePath());
	auto List = path.entryInfoList(QDir::Files);
    for(auto it=List.begin(); it!=List.end(); ++it)
    {
        {
            std::string val=it->fileName().toUpper().toStdString();
            if(val==selectorfilename)
            {
                bool result=sobj.ParseFile(QDir::toNativeSeparators(it->absoluteFilePath()).toStdString());
                if(!result)
                {
                    return false;
                }
                count++;
            }
            else if(val==profilename)
            {
                bool result=pobj.ParseFile(QDir::toNativeSeparators(it->absoluteFilePath()).toStdString());
                if(!result)
                {
                    return false;
                }
                count++;
            }
            else if(val==atmosphfilename)
            {
                bool result=atobj.ParseFile(QDir::toNativeSeparators(it->absoluteFilePath()).toStdString());
                if(!result)
                {
                    return false;
                }
                count++;
            }
            if(count==3)
            {
                break;
            }
        }
    }
    if(count!=3)
    {
        std::cout<<"Can not find file: SELECTOR.IN or ATMOSPH.IN or PROFILE.DAT!"<<std::endl;
        return false;
    }
    return HydrusInputCompresser::Compress(sobj,atobj,pobj,tofilename);
}

bool HydrusInputCompresser::Compress(const SelectorObject& s,const AtmosphObject& a,
                                     const ProfileObject& p,const std::string& tofilename)
{
    using namespace std;
    int offset[3];
    int startoffset=sizeof(int)*3;
    offset[0]=startoffset;
    offset[1]=offset[0]+s.GetLength();
    offset[2]=offset[1]+a.GetLength();
	QFileInfo tpath(tofilename.c_str());
    QDir ptpath=tpath.dir();
    if(!ptpath.exists())
    {
		ptpath.mkpath(ptpath.absolutePath());
    }
    ofstream out(tofilename,ios_base::binary);
    out.write((const char*)offset,startoffset);
    out<<s<<a<<p;
    out.close();
    return true;
}

bool HydrusInputCompresser::UnCompress(const std::string& filename,const std::string& topath)
{
    using namespace std;
	if(!QFileInfo::exists(filename.c_str()))
    {
        std::cout<<filename<<" does not exist!"<<std::endl;
        return false;
    }
	QDir despath(topath.c_str());
	if (!despath.exists())
	{
		despath.mkpath(topath.c_str());
	}
    ifstream in(filename,ios_base::binary);
    int offset[3];
    in.read((char*)offset,sizeof(int)*3);
    SelectorObject sel(in);
    sel.SaveAsSelectorFile(topath);
    AtmosphObject atmos(in);
    atmos.SaveAsAtmosphFile(topath);
    ProfileObject pro(in);
    pro.SaveAsProfileFile(topath);
    in.close();
    return true;
}


std::unique_ptr<SelectorObject> HydrusInputCompresser::ExtractSelector(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[3];
    in.read((char*)offset,sizeof(int)*3);
    std::unique_ptr<SelectorObject> result(new SelectorObject(in));
    in.close();
    return result;
}

std::unique_ptr<AtmosphObject> HydrusInputCompresser::ExtractAtmosph(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[3];
    in.read((char*)offset,sizeof(int)*3);
    in.seekg(offset[1],std::ios_base::beg);
    std::unique_ptr<AtmosphObject> result(new AtmosphObject(in));
    in.close();
    return result;
}

std::unique_ptr<ProfileObject> HydrusInputCompresser::ExtractProfile(const std::string& filename)
{
	if (!QFileInfo::exists(filename.c_str()))
	{
		std::cout << filename << " does not exist!" << std::endl;
		return nullptr;
	}
    std::ifstream in(filename,std::ios_base::binary);
    int offset[3];
    in.read((char*)offset,sizeof(int)*3);
    in.seekg(offset[2],std::ios_base::beg);
    std::unique_ptr<ProfileObject> result(new ProfileObject(in));
    in.close();
    return result;
}
