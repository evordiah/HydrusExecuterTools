
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

#include "AtmosphParser.h"
#include <sstream>
#include <QFileInfo>
#include <string.h>
#include <limits>
#include <fstream>

AtmosphEncoder::AtmosphEncoder(const std::string &filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

bool AtmosphEncoder::ParseFile(const std::string &filename)
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

AtmosphEncoder::AtmosphEncoder()
{
    _maxAL=0;
    _hCrits=0;
}

bool AtmosphEncoder::ParseFile(std::istream &in)
{
    if(!ParseBlockI(in))
    {
        return false;
    }
    return true;
}

std::vector<bool> AtmosphEncoder::GetLogicalOptions(const std::string &val)
{
    std::vector<bool> bVals;
    for(auto c : val)
    {
        if(c=='t' || c=='T')
        {
            bVals.push_back(true);
        }
        else if(c=='f' || c=='F')
        {
            bVals.push_back(false);
        }
    }
    return bVals;
}

bool AtmosphEncoder::ParseBlockI(std::istream &in)
{
    std::string line;
    std::stringstream strbld;

    //ignore first 3 lines
    for(int i=0;i<3;++i)
    {
        in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    //get MaxAl
    getline(in,line);
    strbld.str(line);
    strbld>>_maxAL;
    _data.reset(new float[5*_maxAL]);

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    auto bVals=GetLogicalOptions(line);
    if(bVals.size()<5)
    {
        return false;
    }
    _logicoptions["lDailyVar"]=bVals[0];
    _logicoptions["lSinusVar"]=bVals[1];
    _logicoptions["lLai"]=bVals[2];
    _logicoptions["lBCCycles"]=bVals[3];
    _logicoptions["lIntercep"]=bVals[4];
    //i only consider user input all the data, not hydrus produce the data,
    //so all the options are false
    if(bVals[0] || bVals[1] || bVals[2] ||bVals[3] ||bVals[4])
    {
        return false;
    }

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    strbld.clear();
    strbld.str(line);
    strbld>>_hCrits;

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    //read all the data, the rB, hB and ht should be zero for all days.
    //so i ignore them
    int num=0;
    float tmpf[5];
    float *pos=_data.get();
    while(num<_maxAL && getline(in,line))
    {
        if(line.empty())
            continue;
        strbld.clear();
        strbld.str(line);
        strbld>>tmpf[0];
        strbld>>tmpf[1];
        strbld>>tmpf[2];
        strbld>>tmpf[3];
        strbld>>tmpf[4];
        ::memcpy(pos,tmpf,sizeof(float)*5);
        pos+=5;
        num++;
    }
    if(num<_maxAL)
    {
        return false;
    }
    return true;
}

std::ostream & operator<<(std::ostream &out, const AtmosphEncoder &obj)
{
    out.write((const char*)&obj._maxAL,sizeof(int));
    out.write((const char*)&obj._hCrits,sizeof(float));
    out.write((const char*)obj._data.get(),sizeof(float)*obj._maxAL*5);
    return out;
}
