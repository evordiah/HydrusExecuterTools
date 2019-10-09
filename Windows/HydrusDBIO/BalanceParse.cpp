
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

#include "BalanceParse.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <QFileInfo>
#include <regex>

void BalanceEncoder::ParseFileHead(std::istream& in)
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

bool BalanceEncoder::GetTime(std::istream& in)
{
    std::string line;
    while(getline(in,line))
    {
        if(line.find("Time")!=std::string::npos)
        {
            std::stringstream strbld;
            strbld.str(line.substr(15));
            float time;
            strbld>>time;
            vec_time.push_back(time);
            return true;
        }
        else if(line.find("Calculation")!=std::string::npos)
        {
            std::smatch m;
            std::regex_search(line,m,std::regex("\\s*(\\d+\\.?\\d*)\\s*$"));
            _caltime=stod(m[1].str());
        }
    }
    return false;
}

bool BalanceEncoder::ParseSubregionNum(std::istream& in)
{
    std::string line;
    while(getline(in,line))
    {
        if(line.find("Sub-region")!=std::string::npos)
        {
            if(!_regionnum)
            {
                std::stringstream strbld;
                strbld.str(line.substr(16));
                short temp;
                while(strbld>>temp)
                {
                    _regionnum++;
                }
            }
            return true;
        }
    }
    return false;
}

bool BalanceEncoder::ParsePartData(std::istream& in,int num)
{
    bool result=false;
    std::string line;
    while(getline(in,line))
    {
        //find mark
        if(line.find("[L]")!=std::string::npos)
        {
            result=true;
            break;
        }
    }
    if(!result)
    {
        return result;
    }
    std::stringstream strbld;
    int nline=0;
    do
    {
        strbld.clear();
        strbld.str(line.substr(15));
        float temp;
        while(strbld>>temp)
        {
            vec_data.push_back(temp);
        }
    }
    while(++nline<num && getline(in,line));
    return result;
}

BalanceEncoder::BalanceEncoder(const std::string& filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

BalanceEncoder::~BalanceEncoder()
{
}

bool BalanceEncoder::ParseFile(const std::string &filename)
{
    using namespace std;
	if (!QFileInfo::exists(filename.c_str()))
    {
        cout<<filename<<" does not exist!"<<endl;
        return false;
    }
    ifstream in(filename);
    if(!ParseFile(in))
    {
        cout<<"can not parse file: "<<filename<<std::endl;
        return false;
    }
    return true;
}

bool BalanceEncoder::ParseFile(std::istream &in)
{
    //ignore head
    ParseFileHead(in);
    //Address start time
    GetTime(in);
    //get number of sub-region
    _regionnum=0;
    ParseSubregionNum(in);
    ParsePartData(in,6);
    vec_num.push_back((_regionnum+1)*4+2);

    while(GetTime(in) && ParseSubregionNum(in) && ParsePartData(in,8))
    {
        vec_num.push_back((_regionnum+1)*4+4);
    }
    _timenum=vec_time.size();
    return true;
}

std::ostream& operator<<(std::ostream& out,const BalanceEncoder& Balance)
{
    //save number of sub-regions
    out.write(&Balance._regionnum,1);
    //save number of times
    out.write(&Balance._timenum,1);
    //save each time
    for(auto it=Balance.vec_time.begin(); it!=Balance.vec_time.end(); ++it)
    {
        float temp=*it;
        out.write((char*)&temp,sizeof(float));
    }
    //save data
    unsigned int ncount=0;
    for(auto it=Balance.vec_num.begin(); it!=Balance.vec_num.end(); ++it)
    {
        short temp=*it;
        ncount+=temp;
        out.write((char*)&temp,sizeof(short));
    }

    for(auto it=Balance.vec_data.begin(); it!=Balance.vec_data.end(); ++it)
    {
        float temp=*it;
        out.write((char*)&temp,sizeof(float));
    }
    if(ncount!=Balance.vec_data.size())
    {
        std::cout<<"BalanceEncoder ERROR FORMAT"<<std::endl;
    }

    int s=Balance._DateandTime.size();
    out.write((const char*)&s,sizeof(int));
    out.write(Balance._DateandTime.c_str(),s);
    s=Balance._LUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(Balance._LUnit.c_str(),s);
    s=Balance._TUnit.size();
    out.write((const char*)&s,sizeof(int));
    out.write(Balance._TUnit.c_str(),s);
    out.write((const char*)&Balance._caltime,sizeof(double));
    return out;
}
