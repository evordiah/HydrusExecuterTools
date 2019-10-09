
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

#include "T_LevelObject.h"
#include <sstream>
#include <QDir>
#include <fstream>

T_LevelObject::T_LevelObject(std::istream& in)
{
    int head[2];
    in.read((char*)(head),2*sizeof(int));
    const short linebytes=sizeof(float)*22;
    if(head[1]!=linebytes)
    {
        std::cout<<"T_LevelObject ERROR FORMAT"<<std::endl;
        return;
    }
    int nRec=head[0];
    _nLine=nRec;

    _data=std::unique_ptr<float[]>(new float[nRec*22]);
    char* pDt=(char*)_data.get();
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

T_LevelObject::T_LevelObject(const std::string &filename):T_LevelEncoder(filename)
{
}

void T_LevelObject::SaveAsT_LevelFile(const std::string &path)
{
    using namespace std;
    stringstream strbld;
    string strhead=" ******* Program HYDRUS\n"
                   " ******* \n"
                   " Welcome to HYDRUS-1D                                                    \n"
                   "[DATEANDTIME]\n"
                   " Units: L = [LUNIT]   , T = [TUNIT] , M = mmol \n\n"
                   "       Time          rTop        rRoot        vTop         vRoot        vBot       sum(rTop)   sum(rRoot)    sum(vTop)   sum(vRoot)    sum(vBot)      hTop         hRoot        hBot        RunOff    sum(RunOff)     Volume     sum(Infil)    sum(Evap) TLevel Cum(WTrans)  SnowLayer\n"
                   "        [T]         [L/T]        [L/T]        [L/T]        [L/T]        [L/T]         [L]          [L]          [L]         [L]           [L]         [L]           [L]         [L]          [L/T]         [L]          [L]          [L]          [L]\n";

    auto pos=strhead.find("[DATEANDTIME]");
    strhead=strhead.replace(pos,13,_DateandTime);
    pos=strhead.find("[LUNIT]");
    strhead=strhead.replace(pos,7,_LUnit);
    pos=strhead.find("[TUNIT]");
    strhead=strhead.replace(pos,7,_TUnit);
    strbld<<strhead<<std::endl;

    auto tmp=_data.get();
    for(int i=0; i<_nLine; ++i)
    {
        FormatLine(strbld,tmp);
        tmp+=22;
    }
    strbld<<"end"<<endl;

	QDir p(path.c_str());
	if (!p.exists())
	{
		p.mkpath(path.c_str());
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("T_Level.out")).toStdString();
    std::ofstream out(file);
    out<<strbld.str();
    out.close();
}

std::istream& operator>>(std::istream &in, T_LevelObject &obj)
{
    obj.ParseFile(in);
    return in;
}

std::ostream& operator<<(std::ostream &out, const T_LevelObject &obj)
{
    out<<(T_LevelEncoder&)obj;
    return out;
}

std::string T_LevelObject::FormatFloat(float value)
{
    std::stringstream strbld;
    strbld<<std::scientific;
    strbld.precision(4);
    strbld<<std::uppercase;
    strbld<<value*10;
    std::string svalue=strbld.str();
    strbld.str("");
    float fv;
    std::string postfix;
    if(svalue.size()==10)
    {
        fv=stof(svalue.substr(0,6))/10;
        postfix=svalue.substr(6);
    }
    else
    {
        fv=stof(svalue.substr(0,7))/10;
        postfix=svalue.substr(7);
    }
    strbld.width(9);
    strbld<<std::fixed;
    strbld.precision(5);
    strbld<<fv<<postfix;
    return strbld.str();
}

void T_LevelObject::FormatLine(std::ostream& out, const float *pLine)
{
    out.width(13);
    out<<std::fixed;
    out.precision(4);
    out<<pLine[0];

    for(int i=1; i<19; ++i)
    {
        out<<FormatFloat(pLine[i]);
    }

    out.width(7);
    out<<int(pLine[19]);

    out<<FormatFloat(pLine[20]);

    out.width(11);
    out<<std::fixed;
    out.precision(3);
    out<<pLine[21]<<std::endl;
}


