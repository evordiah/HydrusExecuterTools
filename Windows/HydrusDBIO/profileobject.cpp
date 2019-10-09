
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

#include "profileobject.h"
#include <cstring>
#include <sstream>
#include <fstream>
#include <QDir>

ProfileObject::ProfileObject()
{
}

ProfileObject::ProfileObject(float depth, int nodecnt, float interval, int observercnt, float maxrootdepth)
{
    _depth=depth;
    if(nodecnt>0)
    {
        _nodecnt=nodecnt;
        _interval=_depth/(nodecnt-1);
        AllocateMemory();
    }
    else if(interval>0)
    {
        _interval=interval;
        _nodecnt=_depth/_interval+1;
        AllocateMemory();
    }
    if(observercnt>0)
    {
        _observercnt=observercnt;
        _observeNodeid.reset(new int[_observercnt]);
    }
    else
    {
        _observercnt=0;
        _observeNodeid=nullptr;
    }
    if(_nodecnt>0)
    {
        GenerateCoords();
        GenerateBeta(maxrootdepth,interval);
    }
}

ProfileObject::ProfileObject(const std::string &filename):ProfileEncoder(filename)
{
}

ProfileObject::ProfileObject(std::istream& in)
{
    in.read((char*)&_depth,sizeof(float));
    in.read((char*)&_nodecnt,sizeof(int));
    _interval=_depth/(_nodecnt-1);
    AllocateMemory();
    in.read((char*)&_observercnt,sizeof(int));
    if(_observercnt)
    {
        _observeNodeid.reset(new int[_observercnt]);
        in.read((char*)_observeNodeid.get(),sizeof(int)*_observercnt);
    }
    else
    {
        _observeNodeid=nullptr;
    }
    in.read((char*)_xcoord.get(),sizeof(float)*_nodecnt);
    in.read((char*)_h.get(),sizeof(float)*_nodecnt);
    in.read((char*)_mat.get(),sizeof(int)*_nodecnt);
    in.read((char*)_lay.get(),sizeof(int)*_nodecnt);
    in.read((char*)_beta.get(),sizeof(float)*_nodecnt);
    in.read((char*)_Ah.get(),sizeof(float)*_nodecnt);
    in.read((char*)_Ak.get(),sizeof(float)*_nodecnt);
    in.read((char*)_Ath.get(),sizeof(float)*_nodecnt);
}


ProfileObject::~ProfileObject()
{
}

std::string ProfileObject::GetObservationIDs()
{
    if(_observercnt)
    {
        std::stringstream strbld;
        strbld<<"array[";
        int i=0;
        for(;i<_observercnt-1;++i)
        {
            strbld<<_observeNodeid[i]<<",";
        }
        strbld<<_observeNodeid[i]<<"]";
        return strbld.str();
    }
    return "";
}

void ProfileObject::GenerateCoords()
{
    int i=0;
    for(; i<_nodecnt-1; ++i)
    {
        _xcoord[i]=-_interval*i;
    }
    _xcoord[i]=-_depth;
}

//hydrus will reset the beta values, so any beta value will be ok.
void ProfileObject::GenerateBeta(float maxroot,float interval)
{
    int i=0;
    float inival=maxroot;
    std::memset((void*)_beta.get(),0,sizeof(float)*_nodecnt);
    while(inival>0)
    {
        _beta[i++]=inival/maxroot;
        inival-=interval;
    }
}

std::string ProfileObject::GetHead()
{
    return "Pcp_File_Version=4\n";
}

std::string ProfileObject::Format(float val)
{
    char t[15];
    sprintf(t,"%.6e",val);
    int cnt=strlen(t);
    if(t[0]=='-')
    {
        if(cnt<14)
        {
            t[14]='\0';
            t[13]=t[12];
            t[12]=t[11];
            t[11]='0';
        }
    }
    else
    {
        if(cnt<13)
        {
            t[13]='\0';
            t[12]=t[11];
            t[11]=t[10];
            t[10]='0';
        }
    }
    return std::string(t);
}

std::string ProfileObject::GetMiddleBlock()
{
    std::stringstream strbld;

    strbld.width(5);
    strbld<<2<<std::endl;
    strbld<<"    1  0.000000e+000  1.000000e+000  1.000000e+000\n";

    strbld.width(5);
    strbld<<2;
    strbld.width(15);
    strbld<<Format(-_depth);
    strbld<<"  1.000000e+000  1.000000e+000\n";

    strbld.width(5);
    strbld<<_nodecnt;
    strbld<<"    0    0    1 x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc \n";
    for(int i=0; i<_nodecnt; ++i)
    {
        strbld.width(5);
        strbld<<i+1;

        strbld.width(15);
        strbld<<Format(_xcoord[i]);

        strbld.width(15);
        strbld<<Format(_h[i]);

        strbld.width(5);
        strbld<<_mat[i];

        strbld.width(5);
        strbld<<_lay[i];

        strbld.width(15);
        strbld<<Format(_beta[i]);

        strbld.width(15);
        strbld<<Format(_Ah[i]);

        strbld.width(15);
        strbld<<Format(_Ak[i]);

        strbld.width(15);
        strbld<<Format(_Ath[i]);

        strbld<<std::endl;
    }
    return strbld.str();
}

std::string ProfileObject::GetEnd()
{
    std::stringstream strbld;
    strbld.width(5);
    strbld<<_observercnt<<std::endl;
    for(int i=0; i<_observercnt; ++i)
    {
        strbld.width(5);
        strbld<<_observeNodeid[i];
    }
    strbld<<std::endl;
    return strbld.str();
}

void ProfileObject::SaveAsProfileFile(const std::string& strpath)
{
	QDir p(strpath.c_str());
	if (!p.exists())
	{
		p.mkpath(strpath.c_str());
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("PROFILE.DAT")).toStdString();
    std::ofstream out(file);
    out<<GetHead();
    out<<GetMiddleBlock();
    out<<GetEnd();
    out.close();
}

std::ostream& operator<<(std::ostream& out,const ProfileObject& obj)
{
    out<<(ProfileEncoder&)(obj);
    return out;
}

std::istream& operator>>(std::istream& in, ProfileObject& obj)
{
    obj.ParseFile(in);
    return in;
}

