
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

#include "profiledatabaseobject.h"
#include <string>
#include <regex>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

ProfileDataBaseObject::ProfileDataBaseObject(int gid, QSqlQuery &qry)
{
    std::stringstream strbld;
    //strbld<<"select depth,nodecnt,observercnt,observerid from profilestatistics where gid="<<gid<<";";
    strbld<<"select * from getprofileinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(!qry.next())
    {
        return;
    }
    _depth=qry.value(0).toFloat();
    _nodecnt=qry.value(1).toInt();
    _interval=_depth/(_nodecnt-1);
    AllocateMemory();
    _observercnt=qry.value(2).toInt();
    if(_observercnt)
    {
        _observeNodeid.reset(new int[_observercnt]);
        SetObserverId(qry.value(3).toString().toStdString());
    }
    else
    {
        _observeNodeid=nullptr;
    }
    strbld.str("");
    //strbld<<"select x_coord,matnum,laynum,beta,h,ah,ak,ath from profile where gid="<<gid<<" order by node;";
    strbld<<"select * from getprofiledata("<<gid<<");";
    qry.exec(strbld.str().c_str());

    for(int i=0;i<_nodecnt;++i)
    {
        if(qry.next())
        {
            _xcoord[i]=qry.value(0).toFloat();
            _mat[i]=qry.value(1).toInt();
            _lay[i]=qry.value(2).toInt();
            _beta[i]=qry.value(3).toFloat();
            _h[i]=qry.value(4).toFloat();
            _Ah[i]=qry.value(5).toFloat();
            _Ak[i]=qry.value(6).toFloat();
            _Ath[i]=qry.value(7).toFloat();
        }
    }
    if(_xcoord[0]==0)
        _xcoord[0]*=-1;
}

void ProfileDataBaseObject::SetObserverId(const std::string &value)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value,mat,pattern);
    std::stringstream strbld(mat.str(1));
    std::string singlevalue;
    int i=0;
    while(i<_observercnt && getline(strbld,singlevalue,','))
    {
        _observeNodeid[i++]=std::stoi(singlevalue);
    }
}
