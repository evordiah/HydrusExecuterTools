
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

#include "RuninfoParser.h"
#include <iostream>
#include <sstream>
#include <QDir>
#include <QFileInfo>
#include <fstream>

RuninfoEncoder::RuninfoEncoder(const std::string& filename)
{
    using namespace std;
	QFileInfo file(filename.c_str());
	QDir ppath = file.dir().absolutePath();
	QString errfile = ppath.absoluteFilePath("Error.msg");
	if(QFileInfo::exists(errfile))
    {
        _numNotConvergency=9999;
        return;
    }
	if(!file.exists())
    {
        cout<<filename<<" does not exist!"<<endl;
        return;
    }

    ifstream in(filename);
    stringstream strbld;
    string linecont;
    _numNotConvergency=0;
    while(getline(in,linecont))
    {
        if(linecont.empty())
        {
            continue;
        }
        strbld.clear();
        strbld.str(linecont);
        int extcnt=0;
        float value;
        while(extcnt<7 && strbld)
        {
            strbld>>value;
            extcnt++;
        }
        if(extcnt<7)
        {
            continue;
        }
        string last;
        strbld>>last;
        if(last.size()!=1)
        {
            last=last.substr(last.size()-1);
        }
        if(last!="T" && last!="t")
        {
            _numNotConvergency++;
        }
    }
    in.close();
}

std::ostream& operator<<(std::ostream& out,const RuninfoEncoder& runinfo)
{
    return out.write((const char*)&runinfo._numNotConvergency,sizeof(int));
}

