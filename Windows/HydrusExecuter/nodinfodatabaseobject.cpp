
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

#include "nodinfodatabaseobject.h"
#include <string>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

NodinfoDataBaseObject::NodinfoDataBaseObject(int gid, QSqlQuery &qry)
{
    _nLine=0;
    std::stringstream strbld;
    //strbld<<"set constraint_exclusion = on;"
    //      <<"select tm,count(*) as cnt from nod_info where gid="<<gid
    //     <<" group by tm order by tm;";
    strbld<<"select * from getdesinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(qry.exec(strbld.str().c_str()) && qry.next())
    {
        _TUnit=qry.value(0).toString().toStdString();
        _LUnit=qry.value(1).toString().toStdString();
        _DateandTime=qry.value(2).toString().toStdString();
    }
    strbld.str("");
    strbld<<"select * from getnodinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    while(qry.next())
    {
        vec_time.push_back(qry.value(0).toFloat());
        _nLine+=qry.value(1).toInt();
    }
    _data.reset(new float[11*_nLine]);
    strbld.str("");
    //strbld<<"SELECT node, depth, head, moistrue, k, c, darcian_velocity, sink, kappa, vdivkstop, temp "
    //        " FROM nod_info where gid="<<gid<<" order by tm,node;";
    strbld<<"select * from getnodinfodata("<<gid<<");";
    qry.exec(strbld.str().c_str());
    float * pfv=_data.get();
    for(int i=0;i<_nLine;++i)
    {
        if(qry.next())
        {
            for(int j=0; j<11; ++j)
            {
                pfv[j]=qry.value(j).toFloat();
            }
            pfv+=11;
        }
    }
}
