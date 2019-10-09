
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

#include "selectordatabaseobject.h"
#include <string>
#include <regex>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <memory>

SelectorDataBaseObject::SelectorDataBaseObject(int gid, QSqlQuery &qry)
{
    std::stringstream strbld;
    //strbld<<"SELECT maxtime, nmat, nlay, printinterval, p0, p2h, p2l,"
    //        "p3, printtimes, model, rootgrowthcnt, r2h, r2l, omegac, matdata,"
    //        "printtimedata, rootdate, rootlength,lunit,tunit,"
    //        "lroot,lsink,lwlayer,linitw, maxit,itmin,itmax,"
    //        "tolth,tolh,ha,hb,dt,dtmin,dtmax,dmul,dmul2,inittime,poptm "
    //        "FROM selector where gid="<<gid<<";";
    strbld<<"select * from getselectordata("<<gid<<");";
    qry.exec(strbld.str().c_str());
    if(!qry.next())
    {
        return;
    }
    _maxTime=qry.value(0).toFloat();
    _nMat=qry.value(1).toInt();
    _nLay=qry.value(2).toInt();
    _printInterval=qry.value(3).toFloat();
    _P0=qry.value(4).toFloat();
    _P2H=qry.value(5).toFloat();
    _P2L=qry.value(6).toFloat();
    _P3=qry.value(7).toFloat();
    _printTimes=qry.value(8).toInt();
    vec_printTimedata.reset(new float[_printTimes]);
    _model=qry.value(9).toInt();
    vec_matdata.reset(new float[_nMat*GetSPCnt()]);
    vec_poptmdata.reset(new float[_nMat]);
    int rootcnt=qry.value(10).toInt();
    _r2H=qry.value(11).toFloat();
    _r2L=qry.value(12).toFloat();
    _Omegac=qry.value(13).toFloat();
    std::string smatdata=qry.value(14).toString().toStdString();
    FillMat(smatdata);
    std::string sprinttimes=qry.value(15).toString().toStdString();
    FillPrintTime(sprinttimes);
    if(!qry.value(16).isNull())
    {
        std::string srootday=qry.value(16).toString().toStdString();
        std::string srootlength=qry.value(17).toString().toStdString();
        FillRootGrowth(srootday,srootlength,rootcnt);
    }
    _LUnit=qry.value(18).toString().toStdString();
    _TUnit=qry.value(19).toString().toStdString();
    _lRoot=qry.value(20).toInt();
    _lSink=qry.value(21).toInt();
    _lWlayer=qry.value(22).toInt();
    _lInitW=qry.value(23).toInt();
    _Maxit=qry.value(24).toInt();
    _ItMin=qry.value(25).toInt();
    _ItMax=qry.value(26).toInt();
    _TolTh=qry.value(27).toFloat();
    _TolH=qry.value(28).toFloat();
    _ha=qry.value(29).toFloat();
    _hb=qry.value(30).toFloat();
    _dt=qry.value(31).toFloat();
    _dtMin=qry.value(32).toFloat();
    _dtMax=qry.value(33).toFloat();
    _dMul=qry.value(34).toFloat();
    _dMul2=qry.value(35).toFloat();
    _initTime=qry.value(36).toFloat();
    std::string poptm=qry.value(37).toString().toStdString();
    FillPoptm(poptm);
}

void SelectorDataBaseObject::FillMat(const std::string &value)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value,mat,pattern);
    std::stringstream strbld(mat.str(1));
    int i=0;
    std::string singlevalue;
    int count=_nMat*GetSPCnt();
    while(i<count && getline(strbld,singlevalue,','))
    {
        vec_matdata[i++]=stof(singlevalue);
    }
}

void SelectorDataBaseObject::FillPrintTime(const std::string &value)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value,mat,pattern);
    std::stringstream strbld(mat.str(1));
    int i=0;
    std::string singlevalue;
    while(i<_printTimes && getline(strbld,singlevalue,','))
    {
        vec_printTimedata[i++]=stof(singlevalue);
    }
}

void SelectorDataBaseObject::FillRootGrowth(const std::string &day, const std::string &length,int rootcnt)
{
    std::unique_ptr<float[]> tpflt(new float[rootcnt]);
    std::unique_ptr<float[]> tpflt2(new float[rootcnt]);
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(day,mat,pattern);
    std::stringstream strbld(mat.str(1));

    int i=0;
    std::string singlevalue;
    while(i<rootcnt && getline(strbld,singlevalue,','))
    {
        tpflt[i++]=stof(singlevalue);
    }
    strbld.str("");
    strbld.clear();
    std::regex_search(length,mat,pattern);
    strbld.str(mat.str(1));

    i=0;
    while(i<rootcnt && getline(strbld,singlevalue,','))
    {
        tpflt2[i++]=stof(singlevalue);
    }
    for(int i=0; i<rootcnt; ++i)
    {
        _RootGrowthTable.insert(std::make_pair(tpflt[i],tpflt2[i]));
    }
}

void SelectorDataBaseObject::FillPoptm(const std::string &value)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value,mat,pattern);
    std::stringstream strbld(mat.str(1));
    int i=0;
    std::string singlevalue;
    while(i<_nMat && getline(strbld,singlevalue,','))
    {
        vec_poptmdata[i++]=stof(singlevalue);
    }
}
