
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

#include "T_LevelParser.h"
#include <iostream>
#include <sstream>
#include <QFileInfo>
#include <fstream>
#include <regex>

T_LevelEncoder::T_LevelEncoder(const std::string& filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

T_LevelEncoder::~T_LevelEncoder()
{
}

bool T_LevelEncoder::ParseFile(const std::string &filename)
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

bool T_LevelEncoder::ParseHead(std::istream &in)
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
            return true;
        }
    }
    return false;
}

bool T_LevelEncoder::ParseFile(std::istream &in)
{
    if(!ParseHead(in))
    {
        return false;
    }
    using namespace std;
    //approximate the number of lines according to the filesize and length of per line
    //allocate memory to save time
    auto length=_FileLength;
    int nLine=length/_LineLength;

    _data.reset(new float[nLine*22]);
    auto pnt=_data.get();

    float fval[22];
    _nLine=0;

    stringstream strbld;
    string linecont;
    while(getline(in,linecont))
    {
        if(linecont.empty())
        {
            continue;
        }
        strbld.clear();
        strbld.str(linecont);
        int extcnt=0;
        while(strbld && extcnt<22)
        {
            strbld>>fval[extcnt++];
        }
        if(extcnt<22)
        {
            continue;
        }
        ::memcpy((void*)pnt,(void*)(fval),sizeof(float)*22);
        pnt+=22;
        ++_nLine;

        //if need more memory, then double it
        if(_nLine==nLine)
        {
            nLine*=2;
            std::unique_ptr<float[]> temp(new float[nLine*22]);
            ::memcpy((void*)temp.get(),(void*)_data.get(),_nLine*sizeof(float)*22);
            _data=std::move(temp);
            pnt=_data.get()+_nLine*22;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out,const T_LevelEncoder& T_Level)
{
    out.write((const char*)&T_Level._nLine,sizeof(int));
    int linebytes=sizeof(float)*22;
    out.write((const char*)&linebytes,sizeof(int));
    out.write((const char*)T_Level._data.get(),T_Level._nLine*linebytes);

    int s=T_Level._DateandTime.size();
    out.write((const char*)&s,sizeof(int));
    out.write(T_Level._DateandTime.c_str(),s);
    s=T_Level._LUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(T_Level._LUnit.c_str(),s);
    s=T_Level._TUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(T_Level._TUnit.c_str(),s);
    return out;
}
