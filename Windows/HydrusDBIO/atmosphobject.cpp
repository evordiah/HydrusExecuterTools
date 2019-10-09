
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

#include "atmosphobject.h"
#include <sstream>
#include <fstream>
#include <QDir>

AtmosphObject::AtmosphObject()
{
}

AtmosphObject::AtmosphObject(int maxAL, float hCritS)
{
    _maxAL=maxAL;
    _hCrits=hCritS;
    if(_maxAL>0)
    {
        _data.reset(new float[_maxAL*5]);
    }
    else
    {
        _maxAL=0;
        _data=nullptr;
    }
}

AtmosphObject::AtmosphObject(std::istream& in)
{
    in.read((char*)&_maxAL,sizeof(int));
    in.read((char*)&_hCrits,sizeof(float));
    _data.reset(new float[_maxAL*5]);
    in.read((char*)_data.get(),sizeof(float)*5*_maxAL);
}

AtmosphObject::AtmosphObject(const std::string &filename):AtmosphEncoder(filename)
{
}

AtmosphObject::~AtmosphObject()
{
}

std::string AtmosphObject::GetHead()
{
    return "Pcp_File_Version=4\n";
}

std::string AtmosphObject::GetBlockI()
{
    std::stringstream strbld;
    std::string result="*** BLOCK I: ATMOSPHERIC INFORMATION  **********************************\n"
                       "   MaxAL                    (MaxAL = number of atmospheric data-records)\n";
    strbld<<result;
    strbld.width(7);
    strbld<<_maxAL<<std::endl;
    strbld<<" DailyVar  SinusVar  lLay  lBCCycles lInterc lDummy  lDummy  lDummy  lDummy  lDummy\n";
    strbld<<"       f       f       f       f       f       f       f       f       f       f\n";
    strbld<<" hCritS                 (max. allowed pressure head at the soil surface)\n";
    strbld.width(7);
    strbld<<_hCrits<<std::endl;
    strbld<<"       tAtm        Prec       rSoil       rRoot      hCritA          rB          hB          ht    RootDepth\n";
    for(int i=0; i<_maxAL; ++i)
    {
        strbld.width(11);
        strbld<<_data[i*5];
        for(int j=1; j<5; ++j)
        {
            strbld.width(12);
            strbld<<_data[i*5+j];
        }
        for(int j=0; j<3; ++j)
        {
            strbld.width(12);
            strbld<<0;
        }
        strbld<<" \n";
    }
    return strbld.str();
}

std::string AtmosphObject::GetEnd()
{
    return "end*** END OF INPUT FILE 'ATMOSPH.IN' **********************************\n";
}

void AtmosphObject::SaveAsAtmosphFile(const std::string& strpath)
{
	QDir p(strpath.c_str());
	if (!p.exists())
	{
		if (!p.mkpath(strpath.c_str()))
		{
			return;
		}
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("ATMOSPH.IN")).toStdString();
    std::ofstream out(file);
    out<<GetHead();
    out<<GetBlockI();
    out<<GetEnd();
    out.close();
}

std::ostream& operator<<(std::ostream& out,const AtmosphObject& obj)
{
    out<<(AtmosphEncoder&)obj;
    return out;
}

std::istream& operator>>(std::istream& in, AtmosphObject& obj)
{
    obj.ParseFile(in);
    return in;
}

