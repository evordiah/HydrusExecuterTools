
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

#include "A_LevelParser.h"
#include <iostream>
#include <sstream>
#include <QFileInfo>
#include <fstream>

A_LevelEncoder::A_LevelEncoder(const std::string& filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

A_LevelEncoder::~A_LevelEncoder()
{
}

bool A_LevelEncoder::ParseFile(const std::string &filename)
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

bool A_LevelEncoder::ParseFile(std::istream &in)
{
    //approximate the number of lines according to the filesize and length of per line
    //allocate memory to save time
    auto length=_FileLength;
    int nLine=length/_LineLength;

    _data.reset(new float[nLine*10]);
    auto pnt=_data.get();
    float fval[10];
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
        while(strbld && extcnt<10)
        {
            strbld>>fval[extcnt++];
        }
        if(extcnt<10)
        {
            continue;
        }
        ::memcpy((void*)pnt,(void*)(fval),sizeof(float)*10);
        pnt+=10;
        ++_nLine;

        //if not enough memory, then enlarge it double
        if(_nLine==nLine)
        {
            nLine*=2;
            std::unique_ptr<float[]> temp(new float[nLine*10]);
            ::memcpy((void*)temp.get(),(void*)_data.get(),_nLine*sizeof(float)*10);
            _data=std::move(temp);
            pnt=_data.get()+_nLine*10;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out,const A_LevelEncoder& A_Level)
{
    //record the number of records
    out.write((const char*)&A_Level._nLine,sizeof(int));
    int linebytes=sizeof(float)*10;
    //record the length of a record
    out.write((const char*)&linebytes,sizeof(int));
    return out.write((const char*)A_Level._data.get(),A_Level._nLine*linebytes);
}




