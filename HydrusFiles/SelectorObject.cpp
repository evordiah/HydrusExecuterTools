
/******************************************************************************
 *
 *
 *  Copyright (c) 2020, Wenzhao Feng.
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

#include <fstream>
#include <QString>
#include <QStringList>
#include <sstream>
#include <iomanip>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include <regex>
#include "SelectorObject.h"
#include "HydrusParameterFilesManager.h"

SelectorObject::SelectorObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent=parent;
    Initial();
    _isValid=open(filename);
    if(_parent && _isValid)
    {
        _parent->_NS=NS;
        _parent->_NLayer=NLay;
        _parent->_Hed=_Hed;
        _parent->_LUnit=_LUnit;
        _parent->_TUnit=_TUnit;
        _parent->_MUnit=_TUnit;
        _parent->_status=_status;
        _parent->_CalTm=_CalTm;
    }
}

SelectorObject::SelectorObject(int gid, QSqlQuery &qry, HydrusParameterFilesManager *parent)
{
    _parent=parent;
    Initial();
    _isValid=open(gid,qry);
    if(_parent  && _isValid)
    {
        _parent->_NS=NS;
        _parent->_NLayer=NLay;
        _parent->_NObs=_NObs;
        if(_NObs)
        {
            _parent->_iobs.reset(new int[_NObs]);
            for(int i=0;i<_NObs;++i)
            {
                _parent->_iobs[i]=_iObs[i];
            }
        }
        _parent->_Hed=_Hed;
        _parent->_LUnit=_LUnit;
        _parent->_TUnit=_TUnit;
        _parent->_MUnit=_TUnit;
        _parent->_status=_status;
        _parent->_CalTm=_CalTm;
    }
}

SelectorObject::~SelectorObject()
{

}

bool SelectorObject::Save(const std::string &path)
{
    if(!_isValid)
    {
        return false;
    }
    QString p=path.c_str();
    QDir dir(p);
    if(!dir.exists())
    {
        if(!dir.mkpath(p))
        {
            return false;
        }
    }
    p=dir.absoluteFilePath("SELECTOR.IN");
    std::ofstream out(p.toStdString());
    if(!out)
    {
        return false;
    }
    if(!SaveBlockA(out))
    {
        return false;
    }
    if(!SaveBlockB(out))
    {
        return false;
    }
    if(!SaveBlockC(out))
    {
        return false;
    }
    if(!SaveBlockD(out))
    {
        return false;
    }
    if(!SaveBlockF(out))
    {
        return false;
    }
    if(!SaveBlockG(out))
    {
        return false;
    }
    out.close();
    return true;
}

std::string SelectorObject::ToSqlStatement(const int gid)
{
    if(!_MaxAl)
    {
        return "";
    }
    std::stringstream strbld;
    strbld<<"INSERT INTO public.selector(gid, lunit, tunit, munit, lwat, lchem,"
            "lsink, lroot, lshort, lequil, nmat, nlay, cosalpha,"
            "maxit, tolth, tolh, wlayer, linitw, ha, hb,imodel, matpar,"
            "dt, dtmin,dtmax, dmul, dmul2, itmin, itmax, mpl, tinit,tmax,"
            "lprint, nprintsteps, tprintinterval,tprint,"
            "ngrowth, tgrowth,  rootdepth,"
            "epsi, lupw, lartd, ctola, ctolr, maxitc, pecr, ns, ltort,"
            "lfiltr, nchpar, inonequal, lmassini, leqinit, ltorta, chpar1,"
            "chpar2, chpar3,ktopch, ctop, kbotch, cbot, dsurf, catm, tpulse,"
            "crootmax,omegac, p0, p2h, p2l, p3,  r2h, r2l,  poptm, maxal, hcrits,nobs,iobs,numnp) values (";
    strbld<<gid<<","
         <<"'"<<_LUnit<<"',"
        <<"'"<<_TUnit<<"',"
       <<"'"<<_MUnit<<"',"
      <<std::boolalpha<<lWat<<","
     <<std::boolalpha<<lChem<<","
    <<std::boolalpha<<SinkF<<","
    <<std::boolalpha<<lRoot<<","
    <<std::boolalpha<<ShortO<<","
    <<std::boolalpha<<lEquil<<","
    <<NMat<<","
    <<NLay<<","
    <<CosAlf<<","
    <<MaxIt<<","
    <<TolTh<<","
    <<TolH<<","
    <<std::boolalpha<<WLayer<<","
    <<std::boolalpha<<lInitW<<","
    <<hTab1<<","
    <<hTabN<<","
    <<iModel<<","
    <<ToSqlArray(_MatPars.get(),NMat*6)<<","
    <<dt<<","
    <<dtMin<<","
    <<dtMax<<","
    <<dMul<<","
    <<dMul2<<","
    <<ItMin<<","
    <<ItMax<<","
    <<MPL<<","
    <<tInit<<","
    <<tMax<<","
    <<std::boolalpha<<lPrintD<<","
    <<nPrintSteps<<","
    <<tPrintInterval<<","
    <<ToSqlArray(_TPrint.get(),MPL)<<",";
    if(lRoot)
    {
        strbld<<nGrowth<<","
             <<ToSqlArray(_tGrowth.get(),nGrowth)<<","
            <<ToSqlArray(_RootDepth.get(),nGrowth)<<",";
    }
    else
    {
        strbld<<"null,null,null,";
    }
    if(lChem)
    {
        strbld<<epsi<<","
             <<std::boolalpha<<lUpW<<","
            <<std::boolalpha<<lArtD<<","
           <<cTolA<<","
          <<cTolR<<","
         <<MaxItC<<","
        <<PeCr<<","
        <<NS<<","
        <<std::boolalpha<<lTort<<","
        <<std::boolalpha<<lFiltr<<","
        <<nChPar<<","
        <<iNonEqul<<","
        <<std::boolalpha<<lMassIni<<","
        <<std::boolalpha<<lEqInit<<","
        <<std::boolalpha<<lTortA<<","
        <<ToSqlArray(_ChPart1.get(),NMat*4)<<","
        <<ToSqlArray(_ChPart2.get(),NS*2)<<","
        <<ToSqlArray(_ChPart3.get(),NMat*NS*14)<<","
        <<kTopCh<<","
        <<ToSqlArray(_InitialTopCon.get(),NS)<<","
        <<kBotCh<<","
        <<ToSqlArray(_InitialBotCon.get(),NS)<<",";
        if(kTopCh==-2)
        {
            strbld<<dSurf<<","
                 <<cAtm<<",";
        }
        else
        {
            strbld<<"null,null,";
        }
        strbld<<tPulse<<",";
    }
    else
    {
        strbld<<"null,null,null,null,null,"
                "null,null,null,null,null,"
                "null,null,null,null,null,"
                "null,null,null,null,null,"
                "null,null,null,null,null,";
    }
    if(SinkF)
    {
        strbld<<ToSqlArray(_CRootMax.get(),NS)<<","
             <<OmegaC<<","
            <<P0<<","
           <<P2H<<","
          <<P2L<<","
         <<P3<<","
        <<r2H<<","
        <<r2L<<","
        <<ToSqlArray(_POptm.get(),NMat)<<",";
    }
    else
    {
        strbld<<"null,null,null,null,null"
                "null,null,null,null,";
    }
    strbld<<_MaxAl<<","
         <<_hCritS<<","
        <<_NObs<<","
       <<ToSqlArray(_iObs.get(),_NObs)<<","
      <<_NumNP<<");";
    return strbld.str();
}

bool SelectorObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if(!in)
    {
        return false;
    }
    if(!ReadBlockA(in))
    {
        return false;
    }
    if(!ReadBlockB(in))
    {
        return false;
    }
    if(!ReadBlockC(in))
    {
        return false;
    }
    if(!ReadBlockD(in))
    {
        return false;
    }
    if(!ReadBlockF(in))
    {
        return false;
    }
    if(!ReadBlockG(in))
    {
        return false;
    }
    return true;
}

bool SelectorObject::open(int gid, QSqlQuery &qry)
{
    QString sqlcmd=QString("select lunit, tunit, munit, lwat, lchem,"
                           "lsink, lroot, lshort, lequil, nmat, nlay, cosalpha,"
                           "maxit, tolth, tolh, wlayer, linitw, ha, hb,imodel, matpar,"
                           "dt, dtmin,dtmax, dmul, dmul2, itmin, itmax, mpl, tinit,tmax,"
                           "lprint, nprintsteps, tprintinterval,tprint,"
                           "ngrowth, tgrowth,  rootdepth,"
                           "epsi, lupw, lartd, ctola, ctolr, maxitc, pecr, ns, ltort,"
                           "lfiltr, nchpar, inonequal, lmassini, leqinit, ltorta, chpar1,"
                           "chpar2, chpar3,ktopch, ctop, kbotch, cbot, dsurf, catm, tpulse,"
                           "crootmax,omegac, p0, p2h, p2l, p3,  r2h, r2l,poptm,"
                           "maxal,hcrits,nobs,iobs,numnp,status,caltm "
                           "from selector where gid = %1 ;").arg(gid);
    if(!qry.exec(sqlcmd))
    {
        return false;
    }
    if(!qry.next())
    {
        return false;
    }
    _LUnit=qry.value(0).toString().toStdString();
    _TUnit=qry.value(1).toString().toStdString();
    _MUnit=qry.value(2).toString().toStdString();
    lWat=qry.value(3).toBool();
    lChem=qry.value(4).toBool();
    SinkF=qry.value(5).toBool();
    lRoot=qry.value(6).toBool();
    ShortO=qry.value(7).toBool();
    lEquil=qry.value(8).toBool();
    NMat=qry.value(9).toInt();
    NLay=qry.value(10).toInt();
    CosAlf=qry.value(11).toDouble();
    MaxIt=qry.value(12).toInt();
    TolTh=qry.value(13).toDouble();
    TolH=qry.value(14).toDouble();
    WLayer=qry.value(15).toBool();
    lInitW=qry.value(16).toBool();
    hTab1=qry.value(17).toDouble();
    hTabN=qry.value(18).toDouble();
    iModel=qry.value(19).toInt();
    _MatPars.reset(new double[6*NMat]);
    if(!ParseSqlARRAY(qry.value(20).toString().toStdString(),_MatPars.get(),6*NMat))
    {
        return false;
    }
    dt=qry.value(21).toDouble();
    dtMin=qry.value(22).toDouble();
    dtMax=qry.value(23).toDouble();
    dMul=qry.value(24).toDouble();
    dMul2=qry.value(25).toDouble();
    ItMin=qry.value(26).toInt();
    ItMax=qry.value(27).toInt();
    MPL=qry.value(28).toInt();
    tInit=qry.value(29).toDouble();
    tMax=qry.value(30).toDouble();
    lPrintD=qry.value(31).toBool();
    nPrintSteps=qry.value(32).toInt();
    tPrintInterval=qry.value(33).toDouble();
    if(MPL)
    {
        _TPrint.reset(new double[MPL]);
        if(!ParseSqlARRAY(qry.value(34).toString().toStdString(),_TPrint.get(),MPL))
        {
            return false;
        }
    }
    if(lRoot)
    {
        nGrowth=qry.value(35).toInt();
        _tGrowth.reset(new double[nGrowth]);
        _RootDepth.reset(new double[nGrowth]);
        if(!ParseSqlARRAY(qry.value(36).toString().toStdString(),_tGrowth.get(),nGrowth))
        {
            return false;
        }
        if(!ParseSqlARRAY(qry.value(37).toString().toStdString(),_RootDepth.get(),nGrowth))
        {
            return false;
        }
    }
    if(lChem)
    {
        epsi=qry.value(38).toDouble();
        lUpW=qry.value(39).toBool();
        lArtD=qry.value(40).toBool();
        cTolA=qry.value(41).toDouble();
        cTolR=qry.value(42).toDouble();
        MaxItC=qry.value(43).toInt();
        PeCr=qry.value(44).toDouble();
        NS=qry.value(45).toInt();
        lTort=qry.value(46).toBool();
        lFiltr=qry.value(47).toBool();
        nChPar=qry.value(48).toInt();
        iNonEqul=qry.value(49).toInt();
        lMassIni=qry.value(50).toBool();
        lEqInit=qry.value(51).toBool();
        lTortA=qry.value(52).toBool();
        _ChPart1.reset(new double[NMat*4]);
        if(!ParseSqlARRAY(qry.value(53).toString().toStdString(),_ChPart1.get(),NMat*4))
        {
            return false;
        }
        _ChPart2.reset(new double[NS*2]);
        if(!ParseSqlARRAY(qry.value(54).toString().toStdString(),_ChPart2.get(),NS*2))
        {
            return false;
        }
        _ChPart3.reset(new double[NMat*NS*14]);
        if(!ParseSqlARRAY(qry.value(55).toString().toStdString(),_ChPart3.get(),NMat*NS*14))
        {
            return false;
        }
        kTopCh=qry.value(56).toInt();
        _InitialTopCon.reset(new double[NS]);
        if(!ParseSqlARRAY(qry.value(57).toString().toStdString(),_InitialTopCon.get(),NS))
        {
            return false;
        }
        kBotCh=qry.value(58).toInt();
        _InitialBotCon.reset(new double[NS]);
        if(!ParseSqlARRAY(qry.value(59).toString().toStdString(),_InitialBotCon.get(),NS))
        {
            return false;
        }
        if(kTopCh==-2)
        {
            dSurf=qry.value(60).toDouble();
            cAtm=qry.value(61).toDouble();
        }
        tPulse=qry.value(62).toDouble();
    }
    if(SinkF)
    {
        _CRootMax.reset(new double[NS]);
        if(!ParseSqlARRAY(qry.value(63).toString().toStdString(),_CRootMax.get(),NS))
        {
            return false;
        }
        OmegaC=qry.value(64).toDouble();
        P0=qry.value(65).toDouble();
        P2H=qry.value(66).toDouble();
        P2L=qry.value(67).toDouble();
        P3=qry.value(68).toDouble();
        r2H=qry.value(69).toDouble();
        r2L=qry.value(70).toDouble();
        _POptm.reset(new double[NMat]);
        if(!ParseSqlARRAY(qry.value(71).toString().toStdString(),_POptm.get(),NMat))
        {
            return false;
        }
    }
    _MaxAl=qry.value(72).toInt();
    _hCritS=qry.value(73).toDouble();
    _NObs=qry.value(74).toInt();
    if(_NObs)
    {
        _iObs.reset(new int[_NObs]);
        if(!ParseSqlARRAY(qry.value(75).toString().toStdString(),_iObs.get(),_NObs))
        {
            return false;
        }
    }
    _NumNP=qry.value(76).toInt();
    _status=qry.value(77).toString().toStdString();
    if(!qry.value(78).isNull())
    {
        _CalTm=qry.value(78).toDouble();
    }
    return true;
}

bool SelectorObject::ParseSqlARRAY(const std::string &value,double* p,int nsize)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value,mat,pattern);
    std::stringstream strbld(mat.str(1));
    int i=0;
    std::string singlevalue;
    while(i<nsize && getline(strbld,singlevalue,','))
    {
        p[i++]=stod(singlevalue);
    }
    return true;
}

bool SelectorObject::ParseSqlARRAY(const std::string &value, int *p, int nsize)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value,mat,pattern);
    std::stringstream strbld(mat.str(1));
    int i=0;
    std::string singlevalue;
    while(i<nsize && getline(strbld,singlevalue,','))
    {
        p[i++]=stoi(singlevalue);
    }
    return true;
}

void SelectorObject::UpdateObsInfo()
{
    _parent->_NObs=_NObs;
    if(_NObs)
    {
        _parent->_iobs.reset(new int[_NObs]);
        for(int i=0;i<_NObs;++i)
        {
            _parent->_iobs[i]=_iObs[i];
        }
    }
}

bool SelectorObject::ParseLine(const std::string &line, const std::string &lineformat, const std::vector<void *>& values)
{
    QString l(line.c_str());
    l=l.simplified();
    QStringList lst=l.split(' ');
    QString f(lineformat.c_str());
    QStringList format=f.split(',');
    if(lst.size()<format.size())
    {
        return false;
    }
    for(int i=0;i<format.size();++i)
    {
        if (format[i]=="bool")
        {
            if(lst[i]=="f" || lst[i]=="F")
            {
                *reinterpret_cast<bool*>(values[i])=false;
            }
            else if(lst[i]=="t" || lst[i]=="T")
            {
                *reinterpret_cast<bool*>(values[i])=true;
            }
            else
            {
                return false;
            }
        }
        else if(format[i]=="int")
        {
            bool bok;
            *reinterpret_cast<int*>(values[i])=lst[i].toInt(&bok);
            if(!bok)
            {
                return false;
            }
        }
        else if(format[i]=="double")
        {
            bool bok;
            *reinterpret_cast<double*>(values[i])=lst[i].toDouble(&bok);
            if(!bok)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool SelectorObject::ReadBlockA(std::istream &in)
{
    std::string line;
    //line 0 --HYDRUS-1D version
    std::getline(in,line);
    QString qs(line.c_str());
    qs=qs.trimmed();
    if(qs.startsWith("Pcp_File_Version="))
    {
        double version=qs.mid(17).toDouble();
        if(version<4)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    //line 1-2--Comment lines.
    std::getline(in,line);
    std::getline(in,line);
    //line 3--Heading.
    std::getline(in,line);
    _Hed=QString(line.c_str()).trimmed().toStdString();
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5--Length unit (e.g., 'cm').
    std::getline(in,line);
    _LUnit=line.substr(0,5);
    if(_LUnit.back()=='\r')
    {
        _LUnit.pop_back();
    }
    //line 6--Time unit (e.g., 'min').
    std::getline(in,line);
    _TUnit=line.substr(0,5);
    if(_TUnit.back()=='\r')
    {
        _TUnit.pop_back();
    }
    //line 7--Mass unit for concentration (e.g., 'g', 'mol', '-').
    std::getline(in,line);
    _MUnit=line.substr(0,5);
    if(_MUnit.back()=='\r')
    {
        _MUnit.pop_back();
    }
    //line 8--Comment lines.
    std::getline(in,line);
    //line 9
    std::getline(in,line);
    std::vector<void*> pValue9={&lWat,&lChem,&lTemp,&SinkF,&lRoot,
                                &ShortO,&lWDep,&lScreen,&AtmBC,&lEquil};
    if(!ParseLine(line,"bool,bool,bool,bool,bool,bool,bool,bool,bool,bool",pValue9))
    {
        return false;
    }
    if(lTemp || lWDep || (!AtmBC))
    {
        return false;
    }
    //line 10--Comment line
    std::getline(in,line);
    //line 11
    std::getline(in,line);
    std::vector<void*> pValue11={&lSnow,&lHP1,&lMeteo,&lVapor,&lActRSU,
                                 &lFlux,&lIrrig,&lDummy};
    if(!ParseLine(line,"bool,bool,bool,bool,bool,bool,bool,bool",pValue11))
    {
        return false;
    }
    if(lSnow || lHP1 || lMeteo || lVapor || lActRSU || lIrrig || (!lFlux))
    {
        return false;
    }
    //line 12--Comment line
    std::getline(in,line);
    //line 13
    std::getline(in,line);
    std::vector<void*> pValue13={&NMat,&NLay,&CosAlf};
    if(!ParseLine(line,"int,int,double",pValue13))
    {
        return false;
    }
    return true;
}

bool SelectorObject::ReadBlockB(std::istream &in)
{
    std::string line;
    //line 1-2--Comment lines.
    std::getline(in,line);
    std::getline(in,line);
    //line 3
    std::getline(in,line);
    std::vector<void*> pValue3={&MaxIt,&TolTh,&TolH};
    if(!ParseLine(line,"int,double,double",pValue3))
    {
        return false;
    }
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5
    std::getline(in,line);
    std::vector<void*> pValue5={&TopInF,&WLayer,&KodTop,&lInitW};
    if(!ParseLine(line,"bool,bool,int,bool",pValue5))
    {
        return false;
    }
    if(!TopInF || KodTop!=-1)
    {
        return false;
    }
    //line 6--Comment lines.
    std::getline(in,line);
    //line 7
    std::getline(in,line);
    std::vector<void*> pValue7={&BotInF,&qGWLF,&FreeD,&SeepF,&KodBot,&qDrain,&hSeep};
    if(!ParseLine(line,"bool,bool,bool,bool,int,bool,double",pValue7))
    {
        return false;
    }
    if(BotInF || qGWLF || SeepF || qDrain || (!FreeD))
    {
        return false;
    }
    //line 10--Comment lines.
    std::getline(in,line);
    //line 11
    std::getline(in,line);
    std::vector<void*> pValue11={&hTab1,&hTabN};
    if(!ParseLine(line,"double,double",pValue11))
    {
        return false;
    }
    //line 12--Comment lines.
    std::getline(in,line);
    //line 13
    std::getline(in,line);
    std::vector<void*> pValue13={&iModel,&iHyst};
    if(!ParseLine(line,"int,int",pValue13))
    {
        return false;
    }
    if(iModel==1 || iModel>4 || iHyst>0 )
    {
        return false;
    }
    //line 16--Comment lines.
    std::getline(in,line);
    //line 17
    _MatPars.reset(new double[NMat*6]);
    std::vector<void*> pValue17={nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
    double *pp=_MatPars.get();
    for(int i=0;i<NMat;++i)
    {
        for(int j=0;j<6;j++)
        {
            pValue17[j]=pp+i*6+j;
        }
        std::getline(in,line);
        if(!ParseLine(line,"double,double,double,double,double,double",pValue17))
        {
            return false;
        }
    }
    return true;
}

bool SelectorObject::ReadBlockC(std::istream &in)
{
    std::string line;
    //line 1-2--Comment lines.
    std::getline(in,line);
    std::getline(in,line);
    //line 3
    std::getline(in,line);
    std::vector<void*> pValue3={&dt,&dtMin,&dtMax,&dMul,&dMul2,&ItMin,&ItMax,&MPL};
    if(!ParseLine(line,"double,double,double,double,double,int,int,int",pValue3))
    {
        return false;
    }
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5
    std::getline(in,line);
    std::vector<void*> pValue5={&tInit,&tMax};
    if(!ParseLine(line,"double,double",pValue5))
    {
        return false;
    }
    //line 6--Comment lines.
    std::getline(in,line);
    //line 7
    std::getline(in,line);
    std::vector<void*> pValue7={&lPrintD,&nPrintSteps,&tPrintInterval,&lEnter};
    if(!ParseLine(line,"bool,int,double,bool",pValue7))
    {
        return false;
    }
    if(MPL)
    {
        //line 8--Comment line.
        std::getline(in,line);
        _TPrint.reset(new double[MPL]);
        int i=0;
        while(i<MPL)
        {
            //line 9
            std::getline(in,line);
            QString s(line.c_str());
            s=s.simplified();
            QStringList sl=s.split(' ');
            for(int j=0;j<sl.size();++j)
            {
                bool bok;
                _TPrint[i++]=sl[j].toDouble(&bok);
                if(!bok)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool SelectorObject::ReadBlockD(std::istream &in)
{
    if(!lRoot)
    {
        return true;
    }
    std::string line;
    //line 1-2--Comment lines.
    std::getline(in,line);
    std::getline(in,line);
    //line 3
    std::getline(in,line);
    iRootIn=std::stoi(line);
    if(iRootIn!=1)
    {
        return false;
    }
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5
    std::getline(in,line);
    nGrowth=std::stoi(line);
    //line 6--Comment lines.
    std::getline(in,line);
    //line 7
    _tGrowth.reset(new double[nGrowth]);
    _RootDepth.reset(new double[nGrowth]);
    for(int i=0;i<nGrowth;++i)
    {
        std::getline(in,line);
        QString s(line.c_str());
        s=s.simplified();
        QStringList sl=s.split(' ');
        bool bok1,bok2;
        _tGrowth[i]=sl[0].toDouble(&bok1);
        _RootDepth[i]=sl[1].toDouble(&bok2);
        if(!bok1 || !bok2)
        {
            return false;
        }
    }
    return true;
}

bool SelectorObject::ReadBlockF(std::istream &in)
{
    if(!lChem)
    {
        return true;
    }
    std::string line;
    //line 1-2--Comment lines.
    std::getline(in,line);
    std::getline(in,line);
    //line 3
    std::getline(in,line);
    std::vector<void*> pValue3={&epsi,&lUpW,&lArtD,&lTDep,&cTolA,
                                &cTolR,&MaxItC,&PeCr,&NS,&lTort,
                                &iBact,&lFiltr,&nChPar};
    if(!ParseLine(line,"double,bool,bool,bool,double,"
                  "double,int,double,int,bool,"
                  "int,bool,int",pValue3))
    {
        return false;
    }
    if(lTDep || iBact==1)
    {
        return false;
    }
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5
    std::getline(in,line);
    std::vector<void*> pValue5={&iNonEqul,&lMoistDep,&lDualNEq,&lMassIni,&lEqInit,
                                &lTortA,&lVar};
    if(!ParseLine(line,"int,bool,bool,bool,bool,bool,bool",pValue5))
    {
        return false;
    }
    if(iNonEqul>2 ||  lMoistDep || lDualNEq )
    {
        return false;
    }
    //line 6--Comment lines.
    std::getline(in,line);
    //line 7
    _ChPart1.reset(new double[4*NMat]);
    std::vector<void*> pValue7={nullptr,nullptr,nullptr,nullptr};
    double *pp=_ChPart1.get();
    for(int i=0;i<NMat;++i)
    {
        for(int j=0;j<4;j++)
        {
            pValue7[j]=pp+i*4+j;
        }
        std::getline(in,line);
        if(!ParseLine(line,"double,double,double,double",pValue7))
        {
            return false;
        }
    }
    _ChPart2.reset(new double[2*NS]);
    _ChPart3.reset(new double[14*NMat*NS]);
    for (int i=0;i<NS;++i)
    {
        //line 8--Comment lines.
        std::getline(in,line);
        //line 9
        std::getline(in,line);
        pp=_ChPart2.get()+2*i;
        std::vector<void*> pValue9={pp,pp+1};
        if(!ParseLine(line,"double,double",pValue9))
        {
            return false;
        }
        //line 10--Comment line
        std::getline(in,line);
        //line 11
        pp=_ChPart3.get()+(14*NMat)*i;
        for (int j=0;j<NMat;++j)
        {
            std::getline(in,line);
            QString s(line.c_str());
            s=s.simplified();
            QStringList sl=s.split(' ');
            for(int k=0;k<sl.size();++k)
            {
                bool bok;
                pp[k]=sl[k].toDouble(&bok);
                if(!bok)
                {
                    return false;
                }
            }
            pp+=14;
        }
    }
    //line 23--Comment line
    std::getline(in,line);
    //line 24
    std::getline(in,line);
    _InitialTopCon.reset(new double[NS]);
    _InitialBotCon.reset(new double[NS]);
    QString s(line.c_str());
    s=s.simplified();
    QStringList sl=s.split(' ');
    bool bok;
    int j=0;
    kTopCh=sl[j++].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    for(int i=0;i<NS;++i)
    {
        _InitialTopCon[i]=sl[j++].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    kBotCh=sl[j++].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    for(int i=0;i<NS;++i)
    {
        _InitialBotCon[i]=sl[j++].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    if(kTopCh==-2)
    {
        //line 25--Comment line
        std::getline(in,line);
        //line 26
        std::getline(in,line);
        std::vector<void*> pValue26={&dSurf,&cAtm};
        if(!ParseLine(line,"double,double",pValue26))
        {
            return false;
        }
    }
    //line 27--Comment line
    std::getline(in,line);
    //line 28
    std::getline(in,line);
    tPulse=std::atof(line.c_str());
    return true;
}

bool SelectorObject::ReadBlockG(std::istream &in)
{
    if(!SinkF)
    {
        return true;
    }
    std::string line;
    //line 1-2--Comment lines.
    std::getline(in,line);
    std::getline(in,line);
    //line 3
    if(NS)
    {
        _CRootMax.reset(new double[NS]);
    }
    std::getline(in,line);
    QString s(line.c_str());
    s=s.simplified();
    QStringList sl=s.split(' ');
    bool bok;
    int i=0;
    iMoSink=sl[i++].toInt(&bok);
    if(!bok || iMoSink!=0)
    {
        return false;
    }
    for(int j=0;j<NS;++j)
    {
        bool bok;
        _CRootMax[j]=sl[i++].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    OmegaC=sl[i].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5
    std::getline(in,line);
    std::vector<void*> pValue5={&P0,&P2H,&P2L,&P3,&r2H,&r2L};
    if(!ParseLine(line,"double,double,double,double,double,double",pValue5))
    {
        return false;
    }
    //line 6--Comment lines.
    std::getline(in,line);
    //line 7
    _POptm.reset(new double[NMat]);
    std::getline(in,line);
    QString s1(line.c_str());
    s1=s1.simplified();
    QStringList sl1=s1.split(' ');
    for(int j=0;j<sl1.size();++j)
    {
        bool bok;
        _POptm[j]=sl1[j].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    if(lChem)
    {
        //line 8--Comment lines.
        std::getline(in,line);
        //line 9
        std::getline(in,line);
        QString s(line.c_str());
        s=s.simplified();
        if(s=="f" || s=="F")
        {
            lSolRed=false;
        }
        else
        {
            return false;
        }
    }
    return true;
}

void SelectorObject::Initial()
{
    _Hed="Welcome to HYDRUS-1D (HydrusExecuter)";
    lTemp=false;
    lWDep=false;
    lScreen=false;
    AtmBC=true;
    lInverse=false;
    lSnow=false;
    lHP1=false;
    lMeteo=false;
    lVapor=false;
    lActRSU=false;
    lFlux=true;
    lIrrig=false;
    lDummy=false;
    TopInF=true;
    KodTop=-1;
    BotInF=false;
    qGWLF=false;
    FreeD=true;
    SeepF=false;
    KodBot=-1;
    qDrain=false;
    hSeep=0;
    iHyst=0;
    lEnter=false;
    iRootIn=1;
    lTDep=false;
    iBact=0;
    lMoistDep=false;
    lDualNEq=false;
    lVar=false;
    iMoSink=0;
    lSolRed=false;
    NS=0;
    _MaxAl=0;
    _hCritS=1000;
    _NObs=0;
    _status="todo";
    _CalTm=std::numeric_limits<double>::max();
}

bool SelectorObject::SaveBlockA(std::ostream &out)
{
    //line 0
    out<<"Pcp_File_Version=4"<<std::endl;
    //line 1-2
    out<<"*** BLOCK A: BASIC INFORMATION *****************************************"<<std::endl;
    out<<"Heading"<<std::endl;
    //line 3
    out<<_Hed<<std::endl;
    //line 4
    out<<"LUnit  TUnit  MUnit  (indicated units are obligatory for all input data)"<<std::endl;
    //line 5-7
    out<<_LUnit<<std::endl;
    out<<_TUnit<<std::endl;
    out<<_MUnit<<std::endl;
    //line 8
    out<<"lWat   lChem lTemp  lSink lRoot lShort lWDep lScreen lVariabBC lEquil lInverse"<<std::endl;
    //line 9
    out<<std::setw(3)<<boolalpha(lWat)
      <<std::setw(3)<<boolalpha(lChem)
     <<std::setw(3)<<boolalpha(lTemp)
    <<std::setw(3)<<boolalpha(SinkF)
    <<std::setw(3)<<boolalpha(lRoot)
    <<std::setw(3)<<boolalpha(ShortO)
    <<std::setw(3)<<boolalpha(lWDep)
    <<std::setw(3)<<boolalpha(lScreen)
    <<std::setw(3)<<boolalpha(AtmBC)
    <<std::setw(3)<<boolalpha(lEquil)
    <<std::setw(3)<<boolalpha(lInverse)
    <<std::endl;
    //line 10
    out<<"lSnow  lHP1   lMeteo  lVapor lActiveU lFluxes lIrrig  lDummy  lDummy  lDummy"<<std::endl;
    //line 11
    out<<std::setw(3)<<boolalpha(lSnow)
      <<std::setw(3)<<boolalpha(lHP1)
     <<std::setw(3)<<boolalpha(lMeteo)
    <<std::setw(3)<<boolalpha(lVapor)
    <<std::setw(3)<<boolalpha(lActRSU)
    <<std::setw(3)<<boolalpha(lFlux)
    <<std::setw(3)<<boolalpha(lIrrig)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::endl;
    //line 12
    out<<"NMat    NLay  CosAlpha"<<std::endl;
    out<<std::setw(4)<<NMat<<' '
      <<std::setw(4)<<NLay<<' '
     <<std::setw(13)<<CosAlf<<std::endl;
    return  true;
}

bool SelectorObject::SaveBlockB(std::ostream &out)
{
    //line 1-2
    out<<"*** BLOCK B: WATER FLOW INFORMATION ************************************"<<std::endl;
    out<<"MaxIt   TolTh   TolH       (maximum number of iterations and tolerances)"<<std::endl;
    //line 3
    out<<std::setw(4)<<MaxIt<<' '
      <<std::setw(12)<<TolTh<<' '
     <<std::setw(12)<<TolH
    <<std::endl;
    //line 4
    out<<"TopInf WLayer KodTop InitCond"<<std::endl;
    //line 5
    out<<std::setw(3)<<boolalpha(TopInF)
      <<std::setw(3)<<boolalpha(WLayer)
     <<std::setw(3)<<KodTop
    <<std::setw(3)<<boolalpha(lInitW)
    <<std::endl;
    //line 6
    out<<"BotInf qGWLF FreeD SeepF KodBot DrainF  hSeep"<<std::endl;
    //line 7
    out<<std::setw(3)<<boolalpha(BotInF)
      <<std::setw(3)<<boolalpha(qGWLF)
     <<std::setw(3)<<boolalpha(FreeD)
    <<std::setw(3)<<boolalpha(SeepF)<<' '
    <<std::setw(5)<<KodBot
    <<std::setw(3)<<boolalpha(qDrain)<<' '
    <<std::setw(12)<<hSeep
    <<std::endl;
    //line 10
    out<<"    hTab1   hTabN"<<std::endl;
    //line 11
    out<<std::setw(12)<<hTab1<<' '
      <<std::setw(12)<<hTabN
     <<std::endl;
    //line 12
    out<<"    Model   Hysteresis"<<std::endl;
    //line 13
    out<<std::setw(7)<<iModel<<' '
      <<std::setw(11)<<iHyst
     <<std::endl;
    //line 16
    out<<"   thr     ths    Alfa      n         Ks       l"<<std::endl;
    //line 17
    double *pp=_MatPars.get();
    for(int i=0; i<NMat; ++i)
    {
        for(int j=0;j<6;j++)
        {
            out<<std::setw(12)<<pp[i*6+j]<<" ";
        }
        out<<std::endl;
    }
    return true;
}

bool SelectorObject::SaveBlockC(std::ostream &out)
{
    //line 1-2
    out<<"*** BLOCK C: TIME INFORMATION ******************************************"<<std::endl;
    out<<"        dt       dtMin       dtMax     DMul    DMul2  ItMin ItMax  MPL"<<std::endl;
    //line 3
    out<<std::setw(12)<<dt<<' '
      <<std::setw(12)<<dtMin<<' '
     <<std::setw(12)<<dtMax<<' '
    <<std::setw(12)<<dMul<<' '
    <<std::setw(12)<<dMul2<<' '
    <<std::setw(12)<<ItMin<<' '
    <<std::setw(12)<<ItMax<<' '
    <<std::setw(12)<<MPL<<' '
    <<std::endl;
    //line 4
    out<<"      tInit        tMax"<<std::endl;
    //line 5
    out<<std::setw(12)<<tInit<<' '
      <<std::setw(12)<<tMax<<' '
     <<std::endl;
    //line 6
    out<<"  lPrintD  nPrintSteps tPrintInterval lEnter"<<std::endl;
    //line 7
    out<<std::setw(3)<<boolalpha(lPrintD)<<' '
      <<std::setw(12)<<nPrintSteps<<' '
     <<std::setw(12)<<tPrintInterval<<' '
    <<std::setw(3)<<boolalpha(lEnter)
    <<std::endl;
    //line 8
    out<<"TPrint(1),TPrint(2),...,TPrint(MPL)"<<std::endl;
    //line 9
    int j=0;
    for(int i=0;i<MPL;++i)
    {
        out<<_TPrint[i]<<' ';
        j++;
        if(j==6)
        {
            j=0;
            out<<std::endl;
        }
    }
    if(j)
    {
        out<<std::endl;
    }
    return true;
}

bool SelectorObject::SaveBlockD(std::ostream &out)
{
    if(!lRoot)
    {
        return true;
    }
    //line 1-2
    out<<"*** BLOCK D: ROOT GROWTH INFORMATION ***********************************"<<std::endl;
    out<<"iRootDepthEntry"<<std::endl;
    //line 3
    out<<std::setw(3)<<iRootIn<<std::endl;
    //line 4
    out<<" nGrowth"<<std::endl;
    //line 5
    out<<std::setw(5)<<nGrowth<<std::endl;
    //line 6
    out<<"      Time  RootDepth"<<std::endl;
    //line 7
    for(int i=0;i<nGrowth;++i)
    {
        out<<std::setw(12)<<_tGrowth[i]<<' '
          <<std::setw(12)<<_RootDepth[i]<<std::endl;
    }
    return  true;
}

bool SelectorObject::SaveBlockF(std::ostream &out)
{
    if(!lChem)
    {
        return true;
    }
    //line 1-2
    out<<"*** BLOCK F: SOLUTE TRANSPORT INFORMATION *****************************************************"<<std::endl;
    out<<" Epsi  lUpW  lArtD lTDep    cTolA    cTolR   MaxItC    PeCr  No.Solutes  lTort   iBacter   lFiltr  nChPar"<<std::endl;
    //line 3
    out<<std::setw(12)<<epsi
      <<std::setw(3)<<boolalpha(lUpW)
     <<std::setw(3)<<boolalpha(lArtD)
    <<std::setw(3)<<boolalpha(lTDep)
    <<' '<<cTolA
    <<' '<<cTolR
    <<' '<<MaxItC
    <<' '<<PeCr
    <<' '<<NS
    <<std::setw(3)<<boolalpha(lTort)<<' '
    <<std::setw(3)<<iBact
    <<std::setw(3)<<boolalpha(lFiltr)<<' '
    <<nChPar<<std::endl;
    //line 4
    out<<"iNonEqul lWatDep lDualNEq lInitM  lInitEq lTort lDummy  lDummy  lDummy  lDummy  lCFTr"<<std::endl;
    //line 5
    out<<std::setw(3)<<iNonEqul
      <<std::setw(3)<<boolalpha(lMoistDep)
     <<std::setw(3)<<boolalpha(lDualNEq)
    <<std::setw(3)<<boolalpha(lMassIni)
    <<std::setw(3)<<boolalpha(lEqInit)
    <<std::setw(3)<<boolalpha(lTortA)
    <<std::setw(3)<<boolalpha(lVar)
    <<std::setw(3)<<boolalpha(lVar)
    <<std::setw(3)<<boolalpha(lVar)
    <<std::setw(3)<<boolalpha(lVar)
    <<std::setw(3)<<boolalpha(lVar)
    <<std::endl;
    //line 6
    out<<"     Bulk.d.     DisperL.      Frac      Mobile WC (1..NMat)"<<std::endl;
    //line 7
    for(int i=0;i<NMat;++i)
    {
        int idx=i*4;
        for(int j=0;j<4;++j)
        {
            out<<std::setw(12)<<_ChPart1[idx+j]<<' ';
        }
        out<<std::endl;
    }
    for(int i=0;i<NS;++i)
    {
        //line 8
        out<<"         DifW       DifG                n-th solute"<<std::endl;
        //line 9
        int idx=2*i;
        out<<std::setw(12)<<_ChPart2[idx]<<' '
          <<std::setw(12)<<_ChPart2[idx+1]<<std::endl;
        //line 10
        out<<"Ks          Nu        Beta       Henry       SnkL1       SnkS1       SnkG1       SnkL1'      SnkS1'      SnkG1'      SnkL0       SnkS0       SnkG0        Alfa"<<std::endl;
        //line 11
        idx=14*NMat*i;
        for(int j=0;j<NMat;++j)
        {
            for(int k=0;k<14;++k)
            {
                out<<' '<<_ChPart3[idx++];
            }
            out<<std::endl;
        }
    }
    //line 23
    out<<"      kTopSolute  SolTop    kBotSolute  SolBot"<<std::endl;
    //line 24
    out<<' '<<kTopCh;
    for(int i=0;i<NS;++i)
    {
        out<<' '<<_InitialTopCon[i];
    }
    out<<' '<<kBotCh;
    for(int i=0;i<NS;++i)
    {
        out<<' '<<_InitialBotCon[i];
    }
    out<<std::endl;
    if(kTopCh==-2)
    {
        //line 25
        out<<std::endl;
        //line 26
        out<<' '<<dSurf<<' '<<cAtm<<std::endl;
    }
    //line 27
    out<<"      tPulse"<<std::endl;
    //line 28
    out<<' '<<tPulse<<std::endl;
    return true;
}

bool SelectorObject::SaveBlockG(std::ostream &out)
{
    if(!SinkF)
    {
        return true;
    }
    //line 1-2
    out<<"*** BLOCK G: ROOT WATER UPTAKE INFORMATION *****************************"<<std::endl;
    out<<"     Model  (0 - Feddes, 1 - S shape)  cRootMax    OmegaC"<<std::endl;
    //line 3
    out<<std::setw(3)<<iMoSink;
    for(int i=0;i<NS;++i)
    {
        out<<' '<<_CRootMax[i];
    }
    out<<' '<<OmegaC<<std::endl;
    //line 4
    out<<"       P0       P2H       P2L       P3          r2H        r2L"<<std::endl;
    //line 5
    out<<' '<<P0
      <<' '<<P2H
     <<' '<<P2L
    <<' '<<P3
    <<' '<<r2H
    <<' '<<r2L<<std::endl;
    //line 6
    out<<"POptm(1),POptm(2),...,POptm(NMat)\n";
    //line 7
    for(int i=0;i<NMat;++i)
    {
        out<<' '<<_POptm[i];
    }
    out<<std::endl;
    if(lChem)
    {
        //line 8
        out<<"     Solute Reduction"<<std::endl;
        //line 9
        out<<std::setw(3)<<boolalpha(lSolRed)<<std::endl;
    }
    out<<"*** END OF INPUT FILE 'SELECTOR.IN' ************************************"<<std::endl;
    return true;
}

std::string SelectorObject::ToSqlArray(double *p, int nsize)
{
    std::stringstream strbld;
    if(nsize==0)
    {
        return "null";
    }
    strbld<<"ARRAY[";
    for(int i=0;i<nsize;++i)
    {
        strbld<<p[i]<<",";
    }
    std::string res=strbld.str();
    res.back()=']';
    return res;
}

std::string SelectorObject::ToSqlArray(int *p, int nsize)
{
    std::stringstream strbld;
    if(nsize==0)
    {
        return "null";
    }
    strbld<<"ARRAY[";
    for(int i=0;i<nsize;++i)
    {
        strbld<<p[i]<<",";
    }
    std::string res=strbld.str();
    res.back()=']';
    return res;
}
