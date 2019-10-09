
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

#include "A_LevelObject.h"
#include <sstream>
#include <QDir>
#include <fstream>

A_LevelObject::A_LevelObject(std::istream& in)
{
    using namespace std;
    int head[2];
    in.read((char*)(head),2*sizeof(int));
    const int linebytes=sizeof(float)*10;
    if(head[1]!=linebytes)
    {
        cout<<"A_LevelObject ERROR FORMAT"<<endl;
        return;
    }
    int nRec=head[0];
    _nLine=nRec;
    _data.reset(new float[nRec*10]);

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
}

A_LevelObject::A_LevelObject(const std::string &filename):A_LevelEncoder(filename)
{
}

A_LevelObject::~A_LevelObject()
{

}

void A_LevelObject::SaveAsA_LevelFile(const std::string &path)
{
    using namespace std;
    stringstream strbld;
    strbld<<"\n\n";
    strbld<<"   Time         sum(rTop)     sum(rRoot)    sum(vTop)     sum(vRoot)     sum(vBot)    hTop       hRoot      hBot      A-level"<<endl;
    strbld<<"    [T]           [L]           [L]           [L]           [L]            [L]        [L]         [L]       [L] "<<endl;
    strbld<<"\n";

    auto buf=_data.get();
    for(int i=0; i<_nLine; ++i)
    {
        FormatLine(strbld,buf);
        buf+=10;
    }
    strbld<<"end"<<endl;

	QDir p(path.c_str());
	if (!p.exists())
	{
		if (!p.mkpath(path.c_str()))
		{
			return;
		}
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("A_Level.out")).toStdString();
    std::ofstream out(file);
    out<<strbld.str();
    out.close();
}

std::istream& operator>>(std::istream &in, A_LevelObject &obj)
{
    obj.ParseFile(in);
    return in;
}

std::ostream& operator<<(std::ostream &out, const A_LevelObject &obj)
{
    out<<(A_LevelEncoder&)obj;
    return out;
}

std::string A_LevelObject::FormatFloat(float value)
{
    std::stringstream strbld;
    strbld<<std::scientific;
    strbld.precision(5);
    strbld<<std::uppercase;
    strbld<<value*10;
    std::string svalue=strbld.str();
    strbld.str("");
    float fv;
    std::string postfix;
    if(svalue.size()==11)
    {
        fv=stof(svalue.substr(0,7))/10;
        postfix=svalue.substr(7);
    }
    else
    {
        fv=stof(svalue.substr(0,8))/10;
        postfix=svalue.substr(8);
    }
    strbld.width(10);
    strbld<<std::fixed;
    strbld.precision(6);
    strbld<<fv<<postfix;
    return strbld.str();
}

void A_LevelObject::FormatLine(std::ostream& out, const float* pLine)
{
    out.width(12);
    out<<std::fixed;
    out.precision(5);
    out<<pLine[0];
    for(int i=1; i<6; ++i)
    {
        out<<FormatFloat(pLine[i]);
    }
    for(int i=6; i<9; ++i)
    {
        out.width(11);
        out<<std::fixed;
        out.precision(3);
        out<<pLine[i];
    }
    out.width(8);
    out<<(int)pLine[9]<<std::endl;
}

