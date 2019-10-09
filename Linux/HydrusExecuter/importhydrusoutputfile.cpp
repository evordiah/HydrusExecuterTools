
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

#include "importhydrusoutputfile.h"
#include "A_LevelObject.h"
#include "HydrusResultCompresser.h"
#include "T_LevelObject.h"
#include "NodeInfoObject.h"
#include "BalanceObject.h"
#include "obs_nodeobject.h"
#include <string>
#include <regex>
#include <sstream>
#include <QSqlQuery>

ImportHydrusoutputFile::ImportHydrusoutputFile()
{
    _paobj=nullptr;
    _ptobj=nullptr;
    _pnobj=nullptr;
    _pbobj=nullptr;
    _poobj=nullptr;
    _valid=false;
}

ImportHydrusoutputFile::~ImportHydrusoutputFile()
{
}

bool ImportHydrusoutputFile::Execute(std::shared_ptr<QSqlQuery> qry)
{
    bool result;
    if(!_valid)
    {
        return false;
    }
    try
    {
        result=qry->exec(GetALevelSql());
        result=(result && qry->exec(GetTLevelSql()));
        result=(result && qry->exec(GetNodinfoSql()));
        result=(result && qry->exec(GetBalancesql()));
        if(_poobj)
        {
            result=(result && qry->exec(GetObsNodesql()));
        }
        result=(result && qry->exec(GetUpdateSelectorsql()));
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        return false;
    }
    return result;
}

void ImportHydrusoutputFile::Filename(const std::string &value)
{
    if(!HydrusResultCompresser::IsConverged(value))
    {
        _valid=false;
        return;
    }
    int part=HydrusResultCompresser::GetPartCount(value);
    _paobj= HydrusResultCompresser::ExtractALevel(value);
    _ptobj=HydrusResultCompresser::ExtractTLevel(value);
    _pnobj=HydrusResultCompresser::ExtractNodeInfo(value);
    _pbobj=HydrusResultCompresser::ExtractBalance(value);
    if(part==5)
    {
        _poobj=HydrusResultCompresser::ExtractObsNode(value);
    }

    if(_paobj && _ptobj && _pnobj && _pbobj)
    {
        if(part==4)
        {
            _valid=true;
        }
        else if(_poobj)
        {
            _valid=true;
        }
        else
        {
            _valid=false;
        }
    }
    else
    {
        _valid=false;
    }
}

QString ImportHydrusoutputFile::GetALevelSql()
{
    std::stringstream strbld;
    A_LevelObject* pobj=_paobj.get();

    strbld<<"insert into a_level (gid, tm, sv_top, sv_root, sv_bot,sr_top,sr_root,htop,hroot,hbot,a_level) values ";
    int ncnt=pobj->LineSize();
    for(int i=0;i<ncnt;i++)
    {
        strbld<<"( "<<_gid<<", "<<pobj->Time(i)<<", "
             <<pobj->sum_vTop(i)<<", "<<pobj->sum_vRoot(i)<<", "<<pobj->sum_vBot(i)<<","
            <<pobj->sum_rTop(i)<<","<<pobj->sum_rRoot(i)<<","<<pobj->hTop(i)<<","
           <<pobj->hRoot(i)<<","<<pobj->hBot(i)<<","<<pobj->A_level(i)<<" ),";
    }
    std::string sqlstattment=strbld.str();
    sqlstattment.back()=';';
    return QString(sqlstattment.c_str());
}

QString ImportHydrusoutputFile::GetNodinfoSql()
{
    NodeInfoObject* pobj=_pnobj.get();
    std::stringstream strbld;

    strbld<<"insert into nod_info "
            " (gid, node, darcian_velocity,tm,depth,head,moistrue,k,c,sink,kappa,vdivkstop,temp) values ";
    int tmnum=pobj->TimeNum();
    int ncnt=pobj->NodeNum();
    for(int j=0;j<tmnum;++j)
    {
        float tmv=pobj->Time(j);
        for(int i=0;i<ncnt;i++)
        {
            strbld<<"( "<<_gid<<","<<i+1<<", "<<pobj->Flux(j,i)<<","
                 <<tmv<<","<<pobj->Depth(j,i)<<","<<pobj->Head(j,i)<<","
                <<pobj->Moisture(j,i)<<","<<pobj->K(j,i)<<","<<pobj->C(j,i)<<","
               <<pobj->Sink(j,i)<<","<<pobj->Kappa(j,i)<<","<<pobj->vdivKsTop(j,i)<<","
              <<pobj->Temp(j,i)<<" ),";
        }
    }
    std::string sqlstatement=strbld.str();
    sqlstatement.back()=';';
    return QString(sqlstatement.c_str());
}

QString ImportHydrusoutputFile::GetTLevelSql()
{
    std::stringstream strbld;
    T_LevelObject* pobj=_ptobj.get();

    strbld<<"insert into t_level "
            "(gid, tm, vtop, vroot, vbot, sv_top, sv_root,"
            "sv_bot, vol, s_infil, s_evap,rtop,rroot,sr_top,"
            "sr_root,htop,hroot,hbot,runoff,s_runoff,t_level,"
            "cum_wtrans,snowlayer) values ";

    int ncnt=pobj->LineSize();
    for(int i=0;i<ncnt;i++)
    {
        strbld<<"( "<<_gid<<", "<<pobj->Time(i)<<", "<<pobj->vTop(i)<<", "<<pobj->vRoot(i)<<", "<<pobj->vBot(i)<<", "
             <<pobj->sum_vTop(i)<<", "<<pobj->sum_vRoot(i)<<", "<<pobj->sum_vBot(i)<<", "
            <<pobj->Volume(i)<<", "<<pobj->sum_Infil(i)<<", "<<pobj->sum_Evap(i)<<","
           <<pobj->rTop(i)<<","<<pobj->rRoot(i)<<","<<pobj->sum_rTop(i)<<","<<pobj->sum_rRoot(i)<<","
          <<pobj->hTop(i)<<","<<pobj->hRoot(i)<<","<<pobj->hBot(i)<<","<<pobj->RunOff(i)<<","
         <<pobj->sum_RunOff(i)<<","<<pobj->TLevel(i)<<","<<pobj->Cum_WTrans(i)<<","<<pobj->SnowLayer(i)
        <<"),";
    }

    std::string sqlstatement=strbld.str();
    sqlstatement.back()=';';
    return QString(sqlstatement.c_str());
}

QString ImportHydrusoutputFile::GetBalancesql()
{
    BalanceObject* pobj=_pbobj.get();
    std::stringstream strbld;
    strbld<<"insert into waterbalance (gid ,tm,subregion,length,w_volume,in_flow,"
            "h_mean,top_flux,bot_flux ,watbalt ,watbalr ) values ";
    strbld<<"("<<_gid<<","<< pobj->Time(0)<<","<<0<<","<<pobj->Length(0,0)<<","
         <<pobj->W_volume(0,0)<<","<<pobj->In_flow(0,0)<<","<<pobj->h_Mean(0,0)<<","
        <<pobj->Top_Flux(0)<<","<<pobj->Bot_Flux(0)<<",null,null),";
    for(int i=1;i<=pobj->SugRegionNum();++i)
    {
        strbld<<"("<<_gid<<","<< pobj->Time(0)<<","<<i<<","<<pobj->Length(0,i)<<","
             <<pobj->W_volume(0,i)<<","<<pobj->In_flow(0,i)<<","<<pobj->h_Mean(0,i)<<","
            <<"null,null,null,null),";
    }
    for(unsigned int i=1;i<pobj->TimeNum();++i)
    {
        strbld<<"("<<_gid<<","<< pobj->Time(i)<<","<<0<<","<<pobj->Length(i,0)<<","
             <<pobj->W_volume(i,0)<<","<<pobj->In_flow(i,0)<<","<<pobj->h_Mean(i,0)<<","
            <<pobj->Top_Flux(i)<<","<<pobj->Bot_Flux(i)<<","<<pobj->WatBalT(i)<<","
           <<pobj->WatBalR(i)<<"),";
        for(int j=1;j<=pobj->SugRegionNum();++j)
        {
            strbld<<"("<<_gid<<","<< pobj->Time(i)<<","<<j<<","<<pobj->Length(i,j)<<","
                 <<pobj->W_volume(i,j)<<","<<pobj->In_flow(i,j)<<","<<pobj->h_Mean(i,j)<<","
                <<"null,null,null,null),";
        }
    }

    std::string sqlstatement=strbld.str();
    sqlstatement.back()=';';

    return QString(sqlstatement.c_str());
}

QString ImportHydrusoutputFile::GetObsNodesql()
{
    std::stringstream strbld;
    Obs_NodeObject* pobj=_poobj.get();

    strbld<<"INSERT INTO obsnode "
            "(gid, tm, node, h, theta, flux) values ";

    int ncnt=pobj->LineSize();
    for(int i=0;i<ncnt;i++)
    {
        for(int j=0;j<pobj->NodeCount();++j)
        {
            strbld<<"( "<<_gid<<", "<<pobj->Time(i)<<", "<<pobj->Node(j)<<", "<<pobj->H(i,j)<<", "
                 <<pobj->Theta(i,j)<<", "<<pobj->Flux(i,j)<<"),";
        }
    }

    std::string sqlstatement=strbld.str();
    sqlstatement.back()=';';
    return QString(sqlstatement.c_str());
}

QString ImportHydrusoutputFile::GetUpdateSelectorsql()
{
    BalanceObject* pobj=_pbobj.get();
    std::stringstream strbld;
    strbld<<"select * from updateselectorstatus("
         <<_gid<<",true,'"<<pobj->DateAndTime()<<"',"
        <<pobj->CalculationTime()<<");";
    return QString(strbld.str().c_str());
}
