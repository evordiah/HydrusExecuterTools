
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

#include "obs_nodeparser.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <QFileInfo>
#include <fstream>

Obs_NodeEncoder::Obs_NodeEncoder(const std::string &filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

Obs_NodeEncoder::~Obs_NodeEncoder()
{
}

bool Obs_NodeEncoder::ParseHead(std::istream &in)
{
    bool result=false;
    std::string linecont;
    while(getline(in,linecont))
    {
        if(linecont.empty())
        {
            continue;
        }
        if(linecont.find("Date")!=std::string::npos &&
                linecont.find("Time")!=std::string::npos)
        {
            while(linecont.back()=='\r' || linecont.back()=='\n')
            {
               linecont.pop_back();
            }
            _DateandTime=linecont;
        }
        else if(linecont.find("Units")!=std::string::npos)
        {
            std::smatch m;
            std::regex_search(linecont,m,std::regex("L\\s*=\\s*(.*)\\s*,\\s*T\\s*=\\s*(.*)\\s*,"));
            _LUnit=m[1].str();
            _TUnit=m[2].str();
        }
        else if(linecont.find("Node")!=std::string::npos)
        {
            std::regex reg("Node\\(\\s*(\\d+)\\s*\\)");
            std::smatch matchs;
            auto pos=linecont.cbegin();
            try
            {
                while(std::regex_search(pos,linecont.cend(),matchs,reg))
                {
                    _nodes.push_back(stoi(matchs[1].str()));
                    pos=matchs[0].second;
                }
                result=true;
            }
            catch(...)
            {
                result=false;
            }
            break;
        }
    }
    return result;
}

bool Obs_NodeEncoder::ParseFile(const std::string &filename)
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

bool Obs_NodeEncoder::ParseFile(std::istream &in)
{
    //approximate the number of lines according to the filesize and length of per line
    //allocate memory to save time
    if(!ParseHead(in))
    {
        return false;
    }

    auto length=_FileLength;
    int nLine=length/_LineLength;
    int nFieldCnt=1+_nodes.size()*3;
    _data.reset(new float[nLine*nFieldCnt]);
    auto pnt=_data.get();
    std::unique_ptr<float[]> fval(new float[nFieldCnt]);
    float* pfval=fval.get();
    _nLine=0;

    std::stringstream strbld;
    std::string linecont;
    while(getline(in,linecont))
    {
        if(linecont.empty())
        {
            continue;
        }
        strbld.clear();
        strbld.str(linecont);
        int extcnt=0;
        while(strbld && extcnt<nFieldCnt)
        {
            strbld>>pfval[extcnt++];
        }
        if(extcnt<nFieldCnt)
        {
            continue;
        }
        ::memcpy((void*)pnt,(void*)(pfval),sizeof(float)*nFieldCnt);
        pnt+=nFieldCnt;
        ++_nLine;

        //if not enough memory, then enlarge it double
        if(_nLine==nLine)
        {
            nLine*=2;
            std::unique_ptr<float[]> temp(new float[nLine*nFieldCnt]);
            ::memcpy((void*)temp.get(),(void*)_data.get(),_nLine*sizeof(float)*nFieldCnt);
            _data=std::move(temp);
            pnt=_data.get()+_nLine*nFieldCnt;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out,const Obs_NodeEncoder& obsencoder)
{
    //record the number of records
    out.write((const char*)&obsencoder._nLine,sizeof(int));
    //record the length of a record
    int nNode=obsencoder._nodes.size();
    int linebytes=sizeof(float)*(nNode*3+1);
    out.write((const char*)&linebytes,sizeof(int));
    //record the number of nodes
    out.write((const char*)&nNode,sizeof(int));
    //record all the node index
    int temp;
    for(int i=0;i<nNode;++i)
    {
        temp=obsencoder._nodes[i];
        out.write((const char*)&temp,sizeof(int));
    }
    //record the data
    out.write((const char*)obsencoder._data.get(),obsencoder._nLine*linebytes);

    int s=obsencoder._DateandTime.size();
    out.write((const char*)&s,sizeof(int));
    out.write(obsencoder._DateandTime.c_str(),s);
    s=obsencoder._LUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(obsencoder._LUnit.c_str(),s);
    s=obsencoder._TUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(obsencoder._TUnit.c_str(),s);
    return out;
}
