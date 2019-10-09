
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

#include "ProfileParser.h"
#include <sstream>
#include <vector>
#include <string.h>
#include <limits>
#include <QFileInfo>
#include <fstream>

ProfileEncoder::ProfileEncoder(const std::string &filename)
{
   if(!ParseFile(filename))
   {
       throw std::string("Can not parse file: ")+filename;
   }
}

bool ProfileEncoder::ParseFile(const std::string &filename)
{
    using namespace std;
	if(!QFileInfo::exists(filename.c_str()))
    {
        cout<<filename<<" does not exist!"<<endl;
        return false;
    }
    ifstream in(filename);
    if(!ParseFile(in))
    {
        cout<<"can not parse file: "<<filename<<endl;
        return false;
    }
    return true;
}

bool ProfileEncoder::ParseFile(std::istream &in)
{
    if(!ParseBlockH(in))
    {
        return false;
    }
    return true;
}

std::ostream & operator<<(std::ostream &out, const ProfileEncoder &obj)
{
    out.write((const char*)&obj._depth,sizeof(float));
    out.write((const char*)&obj._nodecnt,sizeof(int));
    out.write((const char*)&obj._observercnt,sizeof(int));
    if(obj._observercnt)
    {
        out.write((const char*)obj._observeNodeid.get(),sizeof(int)*obj._observercnt);
    }
    out.write((const char*)obj._xcoord.get(),sizeof(float)*obj._nodecnt);
    out.write((const char*)obj._h.get(),sizeof(float)*obj._nodecnt);
    out.write((const char*)obj._mat.get(),sizeof(int)*obj._nodecnt);
    out.write((const char*)obj._lay.get(),sizeof(int)*obj._nodecnt);
    out.write((const char*)obj._beta.get(),sizeof(float)*obj._nodecnt);
    out.write((const char*)obj._Ah.get(),sizeof(float)*obj._nodecnt);
    out.write((const char*)obj._Ak.get(),sizeof(float)*obj._nodecnt);
    out.write((const char*)obj._Ath.get(),sizeof(float)*obj._nodecnt);
    return out;
}

ProfileEncoder::ProfileEncoder()
{
    _depth=0;
    _observercnt=0;
    _nodecnt=0;
}

void ProfileEncoder::AllocateMemory()
{
    _xcoord.reset(new float[_nodecnt]);
    _h.reset(new float[_nodecnt]);
    _mat.reset(new int[_nodecnt]);
    _lay.reset(new int[_nodecnt]);
    _beta.reset(new float[_nodecnt]);
    _Ah.reset(new float[_nodecnt]);
    _Ak.reset(new float[_nodecnt]);
    _Ath.reset(new float[_nodecnt]);
}

bool ProfileEncoder::ParseBlockH(std::istream &in)
{
    std::string line;
    std::stringstream strbld;
    //ignore first lines
    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    //get Number of fixed nodes
    getline(in,line);
    strbld.str(line);
    int tmpi;
    strbld>>tmpi;
    //ignore all the fixed nodes, because these nodes have relevant information only for the module PROFILE of
    //the user interface. When the code is used without the user interface, then only
    //two fixed points (top and bottom of the soil profile) with unit nodal density
    //have to be specified.
    for(int i=0;i<tmpi;++i)
    {
        in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    //get Number of nodal points, Number of solutes(should be zero),iTemp (zero),iEquil(should be 1)
    getline(in,line);
    strbld.clear();
    strbld.str(line);
    strbld>>_nodecnt;
    strbld>>tmpi;
    if(tmpi!=0)
    {
        return false;
    }
    strbld>>tmpi;
    if(tmpi!=0)
    {
        return false;
    }
    strbld>>tmpi;
    if(tmpi!=1)
    {
        return false;
    }
    AllocateMemory();

    int num=0;
    while(num<_nodecnt && getline(in,line))
    {
        if(line.empty())
            continue;
        strbld.clear();
        strbld.str(line);
        strbld>>tmpi;
        strbld>>_xcoord[num];
        strbld>>_h[num];
        strbld>>_mat[num];
        strbld>>_lay[num];
        strbld>>_beta[num];
        strbld>>_Ah[num];
        strbld>>_Ak[num];
        strbld>>_Ath[num];
        num++;
    }

    //There are some errors in file format, return
    if(num<_nodecnt)
    {
        return false;
    }

    _depth=-_xcoord[num-1];


    while(getline(in,line))
    {
        if(!line.empty())
            break;
    }

    if(in)
    {
        strbld.clear();
        strbld.str(line);
        strbld>>_observercnt;
        if(_observercnt)
        {
            _observeNodeid.reset(new int[_observercnt]);
        }
        else
        {
            return true;
        }
    }

    while(getline(in,line))
    {
        if(!line.empty())
            break;
    }
    if(in)
    {
        strbld.clear();
        strbld.str(line);
        for(int i=0; i<_observercnt; ++i)
        {
            strbld>>_observeNodeid[i];
        }
    }

    return true;
}
