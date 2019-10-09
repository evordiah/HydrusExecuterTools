
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

#include "atmosphdatabaseobject.h"
#include <string>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

AtmosphDatabaseObject::AtmosphDatabaseObject(int gid, QSqlQuery &qry)
{
    std::stringstream strbld;
    //strbld<<"select recordcnt,hcrits from atmosphstatistics where gid="<<gid<<";";
    strbld<<"select * from getatmosphinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(!qry.next())
    {
        return;
    }
    _maxAL =qry.value(0).toInt();
    _data.reset(new float[_maxAL*5]);
    _hCrits=qry.value(1).toFloat();
    strbld.str("");
    //strbld<<"select tm,prec_cm ,rsoil_cm ,rroot_cm, hcrita from atmosph where gid="<<gid<<" order by tm;";
    strbld<<"select * from getatmosphdata("<<gid<<");";
    qry.exec(strbld.str().c_str());
    for(int i=0;i<_maxAL;++i)
    {
        if(qry.next())
        {
            for(int j=0; j<5; ++j)
            {
                _data[i*5+j]=qry.value(j).toFloat();
            }
        }
    }
}
