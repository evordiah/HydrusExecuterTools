
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

#include "NodeInfoObject.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <QDir>

NodeInfoObject::NodeInfoObject(std::istream& in)
{
    int head[3];
    in.read((char*)(head),3*sizeof(int));

    int nRec=head[0];
    _data.reset(new float[nRec*11]);
    char* pDt=(char*)_data.get();

    const short linebytes=sizeof(float)*11;
    if(head[1]!=linebytes)
    {
        std::cout<<"NodeInfoObject ERROR FORMAT"<<std::endl;
        return;
    }

    int timenum=head[2];
    _NodeNum=nRec/timenum;
    for(int i=0; i<timenum; ++i)
    {
        float t;
        in.read((char*)&t,sizeof(float));
        vec_time.push_back(t);
    }

    const int readRecordNum=100;
    int length=readRecordNum*linebytes;

    int nLine;
    while(in && nRec>0)
    {
        if(nRec<readRecordNum)
        {
            length=nRec*linebytes;
        }
        in.read(pDt,length);
        auto rsize=in.gcount();
        nLine=rsize/linebytes;
        pDt+=rsize;
        nRec-=nLine;
    }

    int s;
    char desinfo[100];
    in.read((char*)(&s),sizeof(int));
    in.read(desinfo,s);
    desinfo[s]=0;
    _DateandTime=desinfo;
    in.read((char*)(&s),sizeof(int));
    in.read(desinfo,s);
    desinfo[s]=0;
    _LUnit=desinfo;
    in.read((char*)(&s),sizeof(int));
    in.read(desinfo,s);
    desinfo[s]=0;
    _TUnit=desinfo;
}

NodeInfoObject::NodeInfoObject(const std::string &filename):Nod_InfEncoder(filename)
{
}

void NodeInfoObject::SaveAsNod_InfFile(const std::string &path)
{
    using namespace std;
    stringstream strbld;
    strbld<<GetFileHead();
    int timenum=vec_time.size();
    int recnumpersection=_nLine/timenum;
    for(int i=0; i<timenum; ++i)
    {
        strbld<<GetSectionHead(i);
        FormatSection(i,recnumpersection,strbld);
        strbld<<"end";
    }
    strbld<<std::endl;
	QDir p(path.c_str());
	if (!p.exists())
	{
		p.mkpath(path.c_str());
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("Nod_Inf.out")).toStdString();
    std::ofstream out(file);
    out<<strbld.str();
    out.close();
}

std::istream& operator>>(std::istream &in, NodeInfoObject &obj)
{
    obj.ParseFile(in);
    return in;
}

std::ostream& operator<<(std::ostream &out, const NodeInfoObject &obj)
{
    out<<(Nod_InfEncoder&)obj;
    return out;
}


std::string NodeInfoObject::FormatFloat(float value,unsigned int precision,int width)
{
    std::stringstream strbld;
    strbld<<std::scientific;
    strbld.precision(precision-1);
    strbld<<std::uppercase;
    strbld<<value*10;
    std::string svalue=strbld.str();
    strbld.str("");
    float fv;
    std::string postfix;
    if(svalue.size()==precision+5)
    {
        fv=stof(svalue.substr(0,precision+1))/10;
        postfix=svalue.substr(precision+1);
    }
    else
    {
        fv=stof(svalue.substr(0,precision+2))/10;
        postfix=svalue.substr(precision+2);
    }
    strbld.width(width-4);
    strbld<<std::fixed;
    strbld.precision(precision);
    strbld<<fv<<postfix;
    return strbld.str();
}

void NodeInfoObject::FormatLine(std::ostream& out,const float* pLine)
{
    out.width(4);
    out<<(short)(pLine[0]);
    out.width(11);
    out<<std::fixed;
    out.precision(4);
    out<<pLine[1];
    out.width(12);
    out<<std::fixed;
    out.precision(3);
    out<<pLine[2];
    out.width(7);
    out<<std::fixed;
    out.precision(4);
    out<<pLine[3];
    out<<FormatFloat(pLine[4],4,13);
    for(int i=5; i<8; ++i)
    {
        out<<FormatFloat(pLine[i],4,12);
    }
    out.width(8);
    out<<(short)(pLine[8]);
    out<<FormatFloat(pLine[9],3,12);
    out.width(8);
    out<<std::fixed;
    out.precision(2);
    out<<pLine[10]<<std::endl;
}

std::string NodeInfoObject::GetFileHead()
{
    std::string head=" ******* Program HYDRUS\n"
                     " ******* \n"
                     " Welcome to HYDRUS-1D                                                    \n"
                     "[DATEANDTIME]\n"
                     " Units: L = [LUNIT]   , T = [TUNIT] , M = mmol \n";
    auto pos=head.find("[DATEANDTIME]");
    head=head.replace(pos,13,_DateandTime);
    pos=head.find("[LUNIT]");
    head=head.replace(pos,7,_LUnit);
    pos=head.find("[TUNIT]");
    head=head.replace(pos,7,_TUnit);
    return head;
}

std::string NodeInfoObject::GetSectionHead(int index)
{
    std::stringstream strbld;
    strbld<<'\n'<<'\n';
    strbld<<" Time:";
    strbld.width(14);
    strbld<<std::fixed;
    strbld.precision(4);
    strbld<<vec_time[index]<<std::endl;
    strbld<<'\n'<<'\n';
    strbld<<" Node      Depth      Head Moisture       K          C         Flux        Sink         Kappa   v/KsTop   Temp\n";
    strbld<<"           [L]        [L]    [-]        [L/T]      [1/L]      [L/T]        [1/T]         [-]      [-]      [C]\n"
          <<std::endl;
    return strbld.str();
}


void NodeInfoObject::FormatSection(int sectioninx,int recnum,std::ostream& out)
{
    auto tmp=_data.get()+sectioninx*recnum*11;
    for(int i=0; i<recnum; ++i)
    {
        FormatLine(out,tmp);
        tmp+=11;
    }
}
