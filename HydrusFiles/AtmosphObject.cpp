
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
#include <sstream>
#include <iomanip>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include "SelectorObject.h"
#include "HydrusParameterFilesManager.h"
#include "AtmosphObject.h"

AtmosphObject::AtmosphObject(const std::string &filename, SelectorObject *sel)
{
    _sel=sel;
    Initial();
    _isValid=open(filename);
    if(_isValid)
    {
        _sel->_MaxAl=_MaxAl;
        _sel->_hCritS=_hCritS;
    }
}

AtmosphObject::AtmosphObject(int gid, QSqlQuery &qry, SelectorObject *sel)
{
    _sel=sel;
    Initial();
    _isValid=open(gid,qry);
}

AtmosphObject::~AtmosphObject()
{

}

bool AtmosphObject::Save(const std::string &path)
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
    p=dir.absoluteFilePath("ATMOSPH.IN");
    std::ofstream out(p.toStdString());
    if(!out)
    {
        return false;
    }
    //line 0
    out<<"Pcp_File_Version=4"<<std::endl;
    //line 1-2
    out<<"*** BLOCK I: ATMOSPHERIC INFORMATION  **********************************"<<std::endl;
    out<<"   MaxAL                    (MaxAL = number of atmospheric data-records)"<<std::endl;
    //line 3
    out<<' '<<_MaxAl<<std::endl;
    //line 4
    out<<" DailyVar  SinusVar  lLay  lBCCycles lInterc lDummy  lDummy  lDummy  lDummy  lDummy"<<std::endl;
    //line 5
    out<<std::setw(3)<<boolalpha(lDailyVar)
      <<std::setw(3)<<boolalpha(lSinusVar)
     <<std::setw(3)<<boolalpha(lLAI)
    <<std::setw(3)<<boolalpha(lBCCycles)
    <<std::setw(3)<<boolalpha(lIntercep)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::setw(3)<<boolalpha(lDummy)
    <<std::endl;
    //line 8
    out<<" hCritS                 (max. allowed pressure head at the soil surface)"<<std::endl;
    //line 9
    out<<std::setw(12)<<_hCritS<<std::endl;

    int NS=_sel->NS;
    if(!NS)
    {
        //line 10
        out<<"       tAtm        Prec       rSoil       rRoot      hCritA          rB          hB          ht    RootDepth"<<std::endl;
        //line 11
        for(int i=0;i<_MaxAl;++i)
        {
            SaveLine(out,i);
        }
    }
    else
    {
        //line 10
        out<<"       tAtm        Prec       rSoil       rRoot      hCritA          rB          hB          ht        tTop        tBot        Ampl        cTop        cBot   RootDepth"<<std::endl;
        //line 11
        for(int i=0;i<_MaxAl;++i)
        {
            SaveLine(out,i,NS);
        }
    }
    //line end
    out<<"end*** END OF INPUT FILE 'ATMOSPH.IN' **********************************"<<std::endl;
    out.close();
    return true;
}

std::string AtmosphObject::ToSqlStatement(const int gid)
{
    std::stringstream strbld;
    strbld<<"INSERT INTO public.atmosph(gid, tatm, prec, rsoil, rroot, hcrita,ht";
    int NS=_sel->NS;
    if(!NS)
    {
        strbld<<") values";
        for(int i=0;i<_MaxAl;++i)
        {
            strbld<<"("<<gid<<","
                 <<_tAtm[i]<<","
                <<_Prec[i]<<","
               <<_rSoil[i]<<","
              <<_rRoot[i]<<","
             <<_hCritA[i]<<","
            <<_hT[i]<<"),";
        }
    }
    else
    {
        strbld<<",ttop,tbot,ampl,";
        for(int i=1;i<=NS;++i)
        {
            strbld<<"ctop"<<i<<",";
        }
        for(int i=1;i<NS;++i)
        {
            strbld<<"cbot"<<i<<",";
        }
        strbld<<"cbot"<<NS<<") values ";
        for(int i=0;i<_MaxAl;++i)
        {
            strbld<<"("<<gid<<","
                 <<_tAtm[i]<<","
                <<_Prec[i]<<","
               <<_rSoil[i]<<","
              <<_rRoot[i]<<","
             <<_hCritA[i]<<","
            <<_hT[i]<<","
            <<_tTop[i]<<","
            <<_tBot[i]<<","
            <<_Ampl[i]<<",";
            double* pp=_cTop.get()+i*NS;
            for(int j=0;j<NS;++j)
            {
                strbld<<pp[j]<<",";
            }
            pp=_cBot.get()+i*NS;
            for(int j=0;j<NS-1;++j)
            {
                strbld<<pp[j]<<",";
            }
            strbld<<pp[NS-1]<<"),";
        }
    }
    std::string sql=strbld.str();
    sql.back()=';';
    return sql;
}

bool AtmosphObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if(!in)
    {
        return false;
    }
    std::string line;
    //line 0--HYDRUS-1D version
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
    //line 3
    std::getline(in,line);
    _MaxAl=std::atoi(line.c_str());
    //line 4--Comment lines.
    std::getline(in,line);
    //line 5
    std::getline(in,line);
    std::vector<void*> pValue5={&lDailyVar,&lSinusVar,&lLAI,
                                &lBCCycles,&lIntercep,&lDummy};
    if(!ParseLine(line,"bool,bool,bool,bool,bool,bool",pValue5))
    {
        return false;
    }
    if(lDailyVar || lSinusVar || lLAI || lBCCycles || lIntercep)
    {
        return false;
    }
    //line 8--Comment lines.
    std::getline(in,line);
    //line 9
    std::getline(in,line);
    _hCritS=std::atof(line.c_str());
    //line 10--Comment lines.
    std::getline(in,line);
    //line 11
    if(!_MaxAl)
    {
        return false;
    }
    _tAtm.reset(new double[_MaxAl]);
    _Prec.reset(new double[_MaxAl]);
    _rSoil.reset(new double[_MaxAl]);
    _rRoot.reset(new double[_MaxAl]);
    _hCritA.reset(new double[_MaxAl]);
    _hT.reset(new double[_MaxAl]);
    int NS=_sel->NS;
    if(NS)
    {
        _tTop.reset(new double[_MaxAl]);
        _tBot.reset(new double[_MaxAl]);
        _Ampl.reset(new double[_MaxAl]);
        _cTop.reset(new double[_MaxAl*NS]);
        _cBot.reset(new double[_MaxAl*NS]);
        for(int i=0;i<_MaxAl;++i)
        {
            std::getline(in,line);
            if(!ParseLine(line,i,NS))
            {
                return false;
            }
        }
    }
    else
    {
        for(int i=0;i<_MaxAl;++i)
        {
            std::getline(in,line);
            if(!ParseLine(line,i))
            {
                return false;
            }
        }
    }
    return true;
}

bool AtmosphObject::open(int gid, QSqlQuery &qry)
{
    _MaxAl=_sel->_MaxAl;
    _hCritS=_sel->_hCritS;
    int NS=_sel->NS;
    _tAtm.reset(new double[_MaxAl]);
    _Prec.reset(new double[_MaxAl]);
    _rSoil.reset(new double[_MaxAl]);
    _rRoot.reset(new double[_MaxAl]);
    _hCritA.reset(new double[_MaxAl]);
    _hT.reset(new double[_MaxAl]);
    std::stringstream strbld;
    strbld<<"select tatm,prec,rsoil,rroot,hcrita,ht";
    if(NS)
    {
        _tTop.reset(new double[_MaxAl]);
        _tBot.reset(new double[_MaxAl]);
        _Ampl.reset(new double[_MaxAl]);
        _cTop.reset(new double[_MaxAl*NS]);
        _cBot.reset(new double[_MaxAl*NS]);
        strbld<<",ttop,tbot,ampl,";
        for(int i=1;i<=NS;++i)
        {
            strbld<<"ctop"<<i<<",";
        }
        for(int i=1;i<NS;++i)
        {
            strbld<<"cbot"<<i<<",";
        }
        strbld<<"cbot"<<NS<<" from atmosph where gid="<<gid<<" order by tatm;";
        if(!qry.exec(strbld.str().c_str()))
        {
            return false;
        }
        int j=0;
        while (qry.next())
        {
            _tAtm[j]=qry.value(0).toDouble();
            _Prec[j]=qry.value(1).toDouble();
            _rSoil[j]=qry.value(2).toDouble();
            _rRoot[j]=qry.value(3).toDouble();
            _hCritA[j]=qry.value(4).toDouble();
            _hT[j]=qry.value(5).toDouble();
            _tTop[j]=qry.value(6).toDouble();
            _tBot[j]=qry.value(7).toDouble();
            _Ampl[j]=qry.value(8).toDouble();
            double* pp=_cTop.get()+j*NS;
            for(int i=0;i<NS;++i)
            {
                pp[i]=qry.value(9+i).toDouble();
            }
            pp=_cBot.get()+j*NS;
            for(int i=0;i<NS;++i)
            {
                pp[i]=qry.value(9+NS+i).toDouble();
            }
            j++;
        }
    }
    else
    {
        strbld<<" from atmosph where gid="<<gid<<" order by tatm;";
        if(!qry.exec(strbld.str().c_str()))
        {
            return false;
        }
        int j=0;
        while (qry.next())
        {
            _tAtm[j]=qry.value(0).toDouble();
            _Prec[j]=qry.value(1).toDouble();
            _rSoil[j]=qry.value(2).toDouble();
            _rRoot[j]=qry.value(3).toDouble();
            _hCritA[j]=qry.value(4).toDouble();
            _hT[j]=qry.value(5).toDouble();
            j++;
        }
    }
    return true;
}

bool AtmosphObject::ParseLine(const std::string &line, const std::string &lineformat, const std::vector<void *> &values)
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

bool AtmosphObject::ParseLine(const std::string &line, const int lineindex)
{
    QString s=QString(line.c_str()).simplified();
    QStringList sl=s.split(' ');
    bool bok;
    _tAtm[lineindex]=sl[0].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Prec[lineindex]=sl[1].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _rSoil[lineindex]=sl[2].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _rRoot[lineindex]=sl[3].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _hCritA[lineindex]=sl[4].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _hT[lineindex]=sl[7].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    return true;
}

bool AtmosphObject::ParseLine(const std::string &line, const int lineindex, const int NS)
{
    QString s=QString(line.c_str()).simplified();
    QStringList sl=s.split(' ');
    bool bok;
    _tAtm[lineindex]=sl[0].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Prec[lineindex]=sl[1].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _rSoil[lineindex]=sl[2].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _rRoot[lineindex]=sl[3].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _hCritA[lineindex]=sl[4].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _hT[lineindex]=sl[7].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _tTop[lineindex]=sl[8].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _tBot[lineindex]=sl[9].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ampl[lineindex]=sl[10].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    double *pp=_cTop.get()+lineindex*NS;
    for(int i=0;i<NS;++i)
    {
        pp[i]=sl[11+i].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    pp=_cBot.get()+lineindex*NS;
    for(int i=0;i<NS;++i)
    {
        pp[i]=sl[11+NS+i].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    return true;
}

void AtmosphObject::Initial()
{
    lDailyVar=false;
    lSinusVar=false;
    lLAI=false;
    lBCCycles=false;
    lIntercep=false;
    lDummy=false;
    _MaxAl=0;
    _hCritS=1000;
}

void AtmosphObject::SaveLine(std::ostream &out, const int lineindex)
{
    out<<' '<<_tAtm[lineindex]
         <<' '<<_Prec[lineindex]
           <<' '<<_rSoil[lineindex]
             <<' '<<_rRoot[lineindex]
               <<' '<<_hCritA[lineindex]
                 <<' '<<0<<' '<<0
                <<' '<<_hT[lineindex]
                  <<std::endl;
}

void AtmosphObject::SaveLine(std::ostream &out,const int lineindex,const int NS)
{
    out<<' '<<_tAtm[lineindex]
         <<' '<<_Prec[lineindex]
           <<' '<<_rSoil[lineindex]
             <<' '<<_rRoot[lineindex]
               <<' '<<_hCritA[lineindex]
                 <<' '<<0<<' '<<0
                <<' '<<_hT[lineindex]
                  <<' '<<_tTop[lineindex]
                    <<' '<<_tBot[lineindex]
                      <<' '<<_Ampl[lineindex];
    double *pp=_cTop.get()+lineindex*NS;
    for(int i=0;i<NS;++i)
    {
        out<<' '<<pp[i];
    }
    pp=_cBot.get()+lineindex*NS;
    for(int i=0;i<NS;++i)
    {
        out<<' '<<pp[i];
    }
    out<<std::endl;
}
