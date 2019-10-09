
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

#include "Nod_InfParser.h"
#include <iostream>
#include <sstream>
#include <QFileInfo>
#include <fstream>
#include <regex>

void Nod_InfEncoder::ParseFileHead(std::istream &in)
{
    std::string line;
    while(getline(in,line))
    {
        if(line.find("Date")!=std::string::npos &&
                line.find("Time")!=std::string::npos)
        {
            while(line.back()=='\r' || line.back()=='\n')
            {
               line.pop_back();
            }
            _DateandTime=line;
        }
        else if(line.find("Units")!=std::string::npos)
        {
            std::smatch m;
            std::regex_search(line,m,std::regex("L\\s*=\\s*(.*)\\s*,\\s*T\\s*=\\s*(.*)\\s*,"));
            _LUnit=m[1].str();
            _TUnit=m[2].str();
            return;
        }
    }
}

bool Nod_InfEncoder::GetTime(std::istream& in)
{
    std::string line;
    while(getline(in,line))
    {
        if(line.find("Time")!=std::string::npos)
        {
            std::stringstream strbld;
            strbld.str(line.substr(7));
            float time;
            strbld>>time;
            vec_time.push_back(time);
            return true;
        }
    }
    return false;
}

void Nod_InfEncoder::ParsePartData(std::istream& in,float** pos,int& nLine)
{
    float fval[11];
    std::string line;
    std::stringstream strbld;

    while(getline(in,line))
    {
        strbld.clear();
        strbld.str(line);
        int extcnt=0;
        while( extcnt<11 && strbld )
        {
            strbld>>fval[extcnt++];
        }
        if(extcnt<11)
        {
            if(line.find("end")!=std::string::npos)
            {
                return;
            }
            continue;
        }
        ::memcpy((void*)(*pos),(void*)(fval),sizeof(float)*11);
        *pos+=11;
        ++_nLine;

        //not enough memory, enlarge to double
        if(_nLine==nLine)
        {
            nLine*=2;
            std::unique_ptr<float[]> temp(new float[nLine*11]);
            ::memcpy((void*)temp.get(),(void*)_data.get(),_nLine*sizeof(float)*11);
            _data=std::move(temp);
            *pos=_data.get()+_nLine*11;
        }
    }
}

bool Nod_InfEncoder::ParseFile(std::istream &in)
{
    using namespace std;
    //approximate lines of file and allocate memory
    auto length=_FileLength;
    int nLine=length/_LineLength;

    _data.reset(new float[nLine*11]);
    auto pnt=_data.get();

    _nLine=0;

    ParseFileHead(in);

    while(GetTime(in))
    {
        ParsePartData(in,&pnt,nLine);
    }
    return true;
}

Nod_InfEncoder::Nod_InfEncoder(const std::string& filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

Nod_InfEncoder::~Nod_InfEncoder()
{
}

bool Nod_InfEncoder::ParseFile(const std::string &filename)
{
    using namespace std;
	if(!QFileInfo::exists(filename.c_str()))
    {
        cout<<filename<<" does not exist!"<<endl;
        return false;
    }
	QFileInfo qfinf(filename.c_str());
	_FileLength = qfinf.size();
    ifstream in(filename);
    if(!ParseFile(in))
    {
        cout<<"can not parse file: "<<filename<<std::endl;
        return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& out,const Nod_InfEncoder& Nod_Inf)
{
    //record number of records
    out.write((const char*)&Nod_Inf._nLine,sizeof(int));
    int linebytes=sizeof(float)*11;
    //record lenght of a record
    out.write((const char*)&linebytes,sizeof(int));
    int timenum=Nod_Inf.vec_time.size();
    //record number of times
    out.write((const char*)&timenum,sizeof(int));
    //record time
    for(unsigned int i=0; i<Nod_Inf.vec_time.size(); ++i)
    {
        float t=Nod_Inf.vec_time[i];
        out.write((const char*)&t,sizeof(float));
    }
    out.write((const char*)Nod_Inf._data.get(),Nod_Inf._nLine*linebytes);

    int s=Nod_Inf._DateandTime.size();
    out.write((const char*)&s,sizeof(int));
    out.write(Nod_Inf._DateandTime.c_str(),s);
    s=Nod_Inf._LUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(Nod_Inf._LUnit.c_str(),s);
    s=Nod_Inf._TUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(Nod_Inf._TUnit.c_str(),s);
    return out;
}
