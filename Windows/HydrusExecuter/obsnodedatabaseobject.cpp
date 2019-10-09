
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

#include "obsnodedatabaseobject.h"
#include <string>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <map>

ObsNodeDataBaseObject::ObsNodeDataBaseObject(int gid, QSqlQuery &qry)
{
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
    strbld<<"select * from getobsnodeinfo("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(!qry.next())
    {
        return;
    }
    std::string val=qry.value(0).toString().toStdString();
    strbld.clear();
    strbld.str(val);
    strbld>>_nLine;
    int nodecnt;
    strbld>>nodecnt;

    _nFiledCnt=nodecnt*3+1;
    _data.reset(new float[_nFiledCnt*_nLine]);

    strbld.clear();
    strbld.str("");
    strbld<<"select * from getobsnodedata("<<gid<<");";
    qry.exec(strbld.str().c_str());

    std::map<float,int> nTm;
    std::map<int,int> nNd;

    int tmIndex=0,tmIndexCnt=0;
    int ndIndex=0,ndIndexCnt=0;

    while(qry.next())
    {
        float tTm=qry.value(0).toFloat();
        int tNd=qry.value(1).toInt();
        float tH=qry.value(2).toFloat();
        float tTheta=qry.value(3).toFloat();
        float tFlux=qry.value(4).toFloat();

        auto ittm=nTm.find(tTm);
        if(ittm==nTm.end())
        {
            tmIndex=tmIndexCnt;
            nTm[tTm]=tmIndexCnt++;
            Time(tmIndex,tTm);
        }
        else
        {
            tmIndex=ittm->second;
        }

        auto itnd=nNd.find(tNd);
        if(itnd==nNd.end())
        {
            ndIndex=ndIndexCnt;
            nNd[tNd]=ndIndexCnt++;
            _nodes.push_back(tNd);
        }
        else
        {
            ndIndex=itnd->second;
        }

        H(tmIndex,ndIndex,tH);
        Theta(tmIndex,ndIndex,tTheta);
        Flux(tmIndex,ndIndex,tFlux);
    }
}
