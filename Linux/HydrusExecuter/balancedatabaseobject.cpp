
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

#include "balancedatabaseobject.h"
#include <string>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <memory>

BalanceDataBaseObject::BalanceDataBaseObject(int gid, QSqlQuery &qry)
{

    _timenum=0;
    _regionnum=0;
    std::stringstream strbld;
    strbld<<"select * from getdesinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(qry.exec(strbld.str().c_str()) && qry.next())
    {
        _TUnit=qry.value(0).toString().toStdString();
        _LUnit=qry.value(1).toString().toStdString();
        _DateandTime=qry.value(2).toString().toStdString();
        _caltime=qry.value(3).toDouble();
    }
    //strbld<<"set constraint_exclusion = on;"
    //      <<"select tm,count(*) as cnt from waterbalance where gid="<<gid
    //     <<" group by tm order by tm;";
    strbld.str("");
    strbld<<"select * from getwaterbalanceinfo("<<gid<<");";
    if(!qry.exec(strbld.str().c_str()))
    {
        return;
    }
    while(qry.next())
    {
       vec_time.push_back(qry.value(0).toFloat());
       _regionnum=qry.value(1).toInt();
       _timenum++;
    }
    _regionnum--;
    int cnt=_regionnum*4;
    vec_num.push_back(cnt+6);
    for(int i=1;i<_timenum;++i)
    {
        vec_num.push_back(cnt+8);
    }
    strbld.str("");
    //strbld<<"SELECT length, w_volume, in_flow, h_mean, top_flux,"
    //        "bot_flux, watbalt, watbalr FROM waterbalance WHERE GID="
    //     <<gid<<" order by tm,subregion;";
    strbld<<"select * from getwaterbalancedata("<<gid<<");";
    qry.exec(strbld.str().c_str());
    cnt=vec_num[0];
    std::unique_ptr<float[]> pfv(new float[cnt]);
    bool bnext=qry.next();
    if(!bnext)
    {
        return;
    }
    pfv[cnt-2]=qry.value(4).toFloat();
    pfv[cnt-1]=qry.value(5).toFloat();
    for(int i=0;i<=_regionnum ;++i)
    {
        if(bnext)
        {
            for(int j=0;j<4;j++)
            {
                pfv[i*4+j]=qry.value(j).toFloat();
            }
        }
        bnext=qry.next();
    }
    for(int i=0;i<4;++i)
    {
        for(int j=0;j<=_regionnum;++j)
        {
            vec_data.push_back(pfv[j*4+i]);
        }
    }
    vec_data.push_back(pfv[cnt-2]);
    vec_data.push_back(pfv[cnt-1]);

    for(int i=1;i<_timenum;i++)
    {
        if(!bnext)
        {
            return;
        }
        cnt=vec_num[i];
        pfv=std::unique_ptr<float[]>(new float[cnt]);
        pfv[cnt-4]=qry.value(4).toFloat();
        pfv[cnt-3]=qry.value(5).toFloat();
        pfv[cnt-2]=qry.value(6).toFloat();
        pfv[cnt-1]=qry.value(7).toFloat();
        for(int i=0;i<=_regionnum ;++i)
        {
            if(bnext)
            {
                for(int j=0;j<4;j++)
                {
                    pfv[i*4+j]=qry.value(j).toFloat();
                }
                bnext=qry.next();
            }
        }
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<=_regionnum;++j)
            {
                vec_data.push_back(pfv[j*4+i]);
            }
        }
        vec_data.push_back(pfv[cnt-4]);
        vec_data.push_back(pfv[cnt-3]);
        vec_data.push_back(pfv[cnt-2]);
        vec_data.push_back(pfv[cnt-1]);
    }
}
