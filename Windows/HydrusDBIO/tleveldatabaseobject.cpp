
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

#include "tleveldatabaseobject.h"
#include <string>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

TLevelDataBaseObject::TLevelDataBaseObject(int gid, QSqlQuery &qry)
{
	_nLine = 0;
    std::stringstream strbld;
    strbld<<"select * from getdesinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(qry.exec(strbld.str().c_str()) && qry.next())
    {
        _TUnit=qry.value(0).toString().toStdString();
        _LUnit=qry.value(1).toString().toStdString();
        _DateandTime=qry.value(2).toString().toStdString();
    }
    strbld.str("");
    //strbld<<"set constraint_exclusion = on;"
    //      <<"select count(*) as cnt from t_level where gid="<<gid<<";";
    strbld<<"select * from gettlevelcount("<<gid<<");";
    if(!qry.exec(strbld.str().c_str()) || !qry.next())
    {
        return;
    }
    _nLine=qry.value(0).toInt();
    _data.reset(new float[22*_nLine]);
    strbld.str("");
    //strbld<<"SELECT tm,rtop,rroot,vtop,vroot, vbot,sr_top, sr_root, sv_top, sv_root, sv_bot,"
    //        "htop, hroot, hbot, runoff,s_runoff,vol, s_infil, s_evap,"
    //        "t_level, cum_wtrans, snowlayer FROM t_level where gid="<<gid<<" order by tm;";

    strbld<<"select * from gettleveldata("<<gid<<");";
    qry.exec(strbld.str().c_str());
    float * pfv=_data.get();
    for(int i=0;i<_nLine;++i)
    {
        if(qry.next())
        {
            for(int j=0; j<22; ++j)
            {
                pfv[j]=qry.value(j).toFloat();
            }
            pfv+=22;
        }
    }
}
