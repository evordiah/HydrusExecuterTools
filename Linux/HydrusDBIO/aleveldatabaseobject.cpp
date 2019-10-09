
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

#include "aleveldatabaseobject.h"
#include <string>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

ALevelDataBaseObject::ALevelDataBaseObject(int gid, QSqlQuery &qry)
{
    _nLine=0;
    std::stringstream strbld;
    //strbld<<"set constraint_exclusion = on;"
    //      <<"select count(*) as cnt from a_level where gid="<<gid<<";";
    strbld<<"select * from getalevelcount("<<gid<<");";
    if(!qry.exec(strbld.str().c_str()) || !qry.next())
    {
        std::cout<<"can not get data when gid = "<<gid<<std::endl;
        return;
    }
    _nLine=qry.value(0).toInt();
    _data.reset(new float[10*_nLine]);
    strbld.str("");
    //strbld<<"SELECT tm,sr_top,sr_root,sv_top, sv_root, sv_bot,  htop, hroot, hbot,a_level "
    //            "FROM a_level where gid="<<gid<<" order by tm;";
    strbld<<"select * from getaleveldata("<<gid<<");";
    qry.exec(strbld.str().c_str());
    float * pfv=_data.get();
    for(int i=0;i<_nLine;++i)
    {
        if(qry.next())
        {
            for(int j=0; j<10; ++j)
            {
                pfv[j]=qry.value(j).toFloat();
            }
            pfv+=10;
        }
    }
}
