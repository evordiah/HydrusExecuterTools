
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

#include "importhydrusinputfile.h"
#include "atmosphobject.h"
#include "HydrusInputCompresser.h"
#include "profileobject.h"
#include "selectorobject.h"
#include <string>
#include <regex>
#include <sstream>
#include <QSqlQuery>

ImportHydrusinputFile::ImportHydrusinputFile()
{
    _paobj=nullptr;
    _ppobj=nullptr;
    _psobj=nullptr;
    _valid=false;
}

ImportHydrusinputFile::~ImportHydrusinputFile()
{
}

bool ImportHydrusinputFile::Execute(std::shared_ptr<QSqlQuery> qry)
{
    if(!_valid)
    {
        return false;
    }
    bool bv1=qry->exec(GetAtmosphsql());
    bool bv2=qry->exec(GetProfilesql());
    bool bv3=qry->exec(GetSelectorsql());
    return bv1 && bv2 && bv3;
}

void ImportHydrusinputFile::Filename(const std::string &value)
{
    _paobj= HydrusInputCompresser::ExtractAtmosph(value);
    _ppobj=HydrusInputCompresser::ExtractProfile(value);
    _psobj=HydrusInputCompresser::ExtractSelector(value);
    if(_paobj && _ppobj && _psobj)
    {
        _valid=true;
    }
    else
    {
        _valid=false;
    }
}

QString ImportHydrusinputFile::GetAtmosphsql()
{
    AtmosphObject* pobj=_paobj.get();
    std::stringstream strbld;

    strbld<<"insert into atmosph (gid, tm, prec_cm, rsoil_cm, rroot_cm, hcrita) values ";
    int ncnt=_paobj->MaxAL();
    for(int i=0;i<ncnt;i++)
    {
        strbld<<"( "<<_gid<<", "<<pobj->TAtm(i)<<", "<<pobj->Prec(i)
             <<", "<<pobj->Evap(i)<<", "<<pobj->Trasp(i)<<", "<<pobj->HCritA(i)<<" ),";
    }
    std::string sqlstatement=strbld.str();
    sqlstatement.back()=';';
    strbld.str("");
    strbld<<"INSERT INTO atmosphstatistics(gid, recordcnt, hcrits) VALUES (";
    strbld<<_gid<<","<<pobj->MaxAL()<<","<<pobj->HCritS()<<");";
    sqlstatement+=strbld.str();
    return QString(sqlstatement.c_str());
}

QString ImportHydrusinputFile::GetProfilesql()
{
    ProfileObject* pobj=_ppobj.get();
    std::stringstream strbld;

    strbld<<"insert into profile (gid, node, x_coord,beta,matnum,laynum,h,ah,ak,ath) values ";

    int ncnt=pobj->NodeCount();
    for(int i=0;i<ncnt;i++)
    {
        strbld<<"("<<_gid<<","<<i+1<<","<<pobj->Coord(i)<<","<<pobj->Beta(i)<<","
             <<pobj->Mat(i)<<","<<pobj->Lay(i)<<","<<pobj->H(i)<<","
            <<pobj->Ah(i)<<","<<pobj->Ak(i)<<","<<pobj->Ath(i)<<"),";
    }
    std::string sqlstatement=strbld.str();
    sqlstatement.back()=';';
    strbld.str("");
    if(pobj->ObserversationCount())
    {
        strbld<<"insert into profilestatistics(gid,depth,nodecnt,observercnt,observerid) values ";
        strbld<<"("<<_gid<<","<<pobj->Depth()<<","<<pobj->NodeCount()<<","<<pobj->ObserversationCount()<<","
             <<pobj->GetObservationIDs()<<");";
    }
    else
    {
        strbld<<"insert into profilestatistics(gid,depth,nodecnt,observercnt) values ";
        strbld<<"("<<_gid<<","<<pobj->Depth()<<","<<pobj->NodeCount()<<", 0 );";
    }
    sqlstatement+=strbld.str();
    return QString(sqlstatement.c_str());
}

QString ImportHydrusinputFile::GetSelectorsql()
{
    SelectorObject* pobj=_psobj.get();
    std::stringstream strbld;
    strbld<<"INSERT INTO selector("
            "gid, maxtime, nmat, nlay, printinterval, p0, p2h, p2l,"
            "p3, printtimes, model, rootgrowthcnt, r2h, r2l,  omegac, matdata,"
            "printtimedata, rootdate, rootlength,lunit,tunit,"
            "lroot,lsink,lwlayer,linitw, maxit,itmin,itmax,"
            "tolth,tolh,ha,hb,dt,dtmin,dtmax,dmul,dmul2,inittime,poptm "
            ") VALUES (";
    strbld<<_gid<<","<<pobj->MaxTime()<<","<<pobj->nMat()<<","
         <<pobj->nLay()<<","<<pobj->PrintInterval()<<","<<pobj->P0()<<","
        <<pobj->P2H()<<","<<pobj->P2L()<<","<<pobj->P3()<<","<<pobj->PrintTimes()<<","
       <<(int)pobj->SPModel()<<","<<pobj->GetRootGrowthCnt()<<","<<pobj->r2H()<<","<<pobj->r2L()<<","
      <<pobj->Omegac()<<","<<pobj->GetMatData()<<","<<pobj->GetPrintTime()<<","
     <<pobj->GetRootGrowthDays()<<","<<pobj->GetRootLengthData()<<",'"
    <<pobj->LUnit()<<"','"<<pobj->TUnit()<<"',"<<pobj->LRoot()<<","<<pobj->LSink()<<","
    <<pobj->LWlayer()<<","<<pobj->LInitW()<<","<<pobj->Maxit()<<","<<pobj->ItMin()<<","
    <<pobj->ItMax()<<","<<pobj->TolTh()<<","<<pobj->TolH()<<","<<pobj->HA()<<","<<pobj->HB()<<","
    <<pobj->DT()<<","<<pobj->DtMin()<<","<<pobj->DtMax()<<","<<pobj->DMul()<<","<<pobj->DMul2()<<","
    <<pobj->InitTime()<<","<<pobj->GetPoptm()<<");";
    return QString(strbld.str().c_str());
}
