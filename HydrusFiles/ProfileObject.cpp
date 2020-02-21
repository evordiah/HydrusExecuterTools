
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
#include "SelectorObject.h"
#include "FFmt.h"
#include "HydrusParameterFilesManager.h"
#include "ProfileObject.h"

ProfileObject::ProfileObject(const std::string &filename, SelectorObject *sel)
{
    _sel=sel;
    Initial();
    _isValid=open(filename);
    if(_isValid)
    {
        _sel->_NumNP=_NumNP;
        _sel->_NObs=_NObs;
        if(_NObs)
        {
            _sel->_iObs.reset(new int[_NObs]);
            for(int i=0;i<_NObs;++i)
            {
                _sel->_iObs[i]=_iObs[i];
            }
        }
        _sel->UpdateObsInfo();
    }
}

ProfileObject::ProfileObject(int gid, QSqlQuery &qry, SelectorObject *sel)
{
    _sel=sel;
    Initial();
    _isValid=open(gid,qry);
    if(_isValid)
    {
        _iTemp=_sel->lChem?1:0;
        if(!_sel->lChem || _sel->lEquil )
        {
            _iEquil=1;
        }
        else
        {
            _iEquil=0;
        }
    }
}

ProfileObject::~ProfileObject()
{

}

bool ProfileObject::Save(const std::string &path)
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
    p=dir.absoluteFilePath("PROFILE.DAT");
    std::ofstream out(p.toStdString());
    if(!out)
    {
        return false;
    }
    //line 0
    out<<"Pcp_File_Version=4"<<std::endl;
    //line 1
    out<<2<<std::endl;
    //line 2
    out<<"    1  0.000000e+000  1.000000e+000  1.000000e+000"<<std::endl;
    auto precision=out.precision();
    out<<"    2"<<std::setw(15)<<std::fixed<<std::setprecision(6)<<fwzformat::SE3<<_x[_NumNP-1]<<"  1.000000e+000  1.000000e+000"<<std::endl;
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    //line 3
    out<<' '<<_NumNP
      <<' '<<_NS
     <<' '<<_iTemp
    <<' '<<_iEquil;
    if(!_iEquil)
    {
        out<<" x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc           SConc";
    }
    else
    {
        out<<" x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc ";
    }
    out<<std::endl;
    //line 4
    if(!_NS)
    {
        for(int i=0;i<_NumNP;++i)
        {
            SaveLine(out,i);
        }
    }
    else
    {
        for(int i=0;i<_NumNP;++i)
        {
            SaveLine(out,i,_NS);
        }
    }
    //line 5
    out<<std::setw(5)<<_NObs<<std::endl;
    //line 6
    for(int i=0;i<_NObs;++i)
    {
        out<<std::setw(4)<<_iObs[i]<<' ';
    }
    out<<std::endl;
    out.close();
    return true;
}

std::string ProfileObject::ToSqlStatement(const int gid)
{
    std::stringstream strbld;
    strbld<<"INSERT INTO public.profile(gid, n, x, hnew, matnum, laynum,beta,ah,ak,ath";
    if(!_NS)
    {
        strbld<<") values";
        for(int i=0;i<_NumNP;++i)
        {
            strbld<<"("<<gid<<","
                 <<_n[i]<<","
                <<_x[i]<<","
               <<_hNew[i]<<","
              <<_MatNum[i]<<","
             <<_LayNum[i]<<","
            <<_Beta[i]<<","
            <<_Ah[i]<<","
            <<_Ak[i]<<","
            <<_Ath[i]<<"),";
        }
    }
    else
    {
        strbld<<",";
        for(int i=1;i<_NS;++i)
        {
            strbld<<"conc"<<i<<",";
        }
        if(_sel->lEquil)
        {
            strbld<<"conc"<<_NS<<") values ";
        }
        else
        {
            strbld<<"conc"<<_NS<<",";
            for(int i=1;i<_NS;++i)
            {
                strbld<<"sorb"<<i<<",";
            }
            strbld<<"sorb"<<_NS<<") values ";
        }
        for(int i=0;i<_NumNP;++i)
        {
            strbld<<"("<<gid<<","
                 <<_n[i]<<","
                <<_x[i]<<","
               <<_hNew[i]<<","
              <<_MatNum[i]<<","
             <<_LayNum[i]<<","
            <<_Beta[i]<<","
            <<_Ah[i]<<","
            <<_Ak[i]<<","
            <<_Ath[i]<<",";
            double* pp=_Conc.get()+i*_NS;
            for(int j=0;j<_NS-1;++j)
            {
                strbld<<pp[j]<<",";
            }
            if(_sel->lEquil)
            {
                strbld<<pp[_NS-1]<<"),";
            }
            else
            {
                strbld<<pp[_NS-1]<<",";
                pp=_Sorb.get()+i*_NS;
                for(int j=0;j<_NS-1;++j)
                {
                    strbld<<pp[j]<<",";
                }
                strbld<<pp[_NS-1]<<"),";
            }
        }
    }
    std::string sql=strbld.str();
    sql.back()=';';
    return sql;
}

bool ProfileObject::open(const std::string &filename)
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
    //line 1
    std::getline(in,line);
    int nFix=std::stoi(line);
    //line 2
    for(int i=0;i<nFix;++i)
    {
        std::getline(in,line);
    }
    //line 3
    std::getline(in,line);
    std::vector<void*> pValue3={&_NumNP,&_NS,&_iTemp,&_iEquil};
    if(!ParseLine(line,"int,int,int,int",pValue3))
    {
        return false;
    }
    if(_NS!=_sel->NS)
    {
        return false;
    }
    if(_sel->lChem)
    {
        if(_iTemp!=1)
        {
            return false;
        }
    }
    else
    {
        if(_iEquil!=1)
        {
            return false;
        }
    }
    if(_sel->lEquil)
    {
        if(_iEquil!=1)
        {
            return false;
        }
    }
    else
    {
        if(_iEquil!=0)
        {
            return false;
        }
    }
    _n.reset(new int[_NumNP]);
    _x.reset(new double[_NumNP]);
    _hNew.reset(new double[_NumNP]);
    _MatNum.reset(new int[_NumNP]);
    _LayNum.reset(new int[_NumNP]);
    _Beta.reset(new double[_NumNP]);
    _Ah.reset(new double[_NumNP]);
    _Ak.reset(new double[_NumNP]);
    _Ath.reset(new double[_NumNP]);
    if(_sel->lChem)
    {
        _Conc.reset(new double[_NumNP*_NS]);
    }
    if(!_sel->lEquil)
    {
        _Sorb.reset(new double[_NumNP*_NS]);
    }
    //line 4
    if(_NS)
    {
        for(int i=0;i<_NumNP;++i)
        {
            std::getline(in,line);
            if(!ParseLine(line,i,_NS))
            {
                return false;
            }
        }
    }
    else
    {
        for(int i=0;i<_NumNP;++i)
        {
            std::getline(in,line);
            if(!ParseLine(line,i))
            {
                return false;
            }
        }
    }
    //line 5
    std::getline(in,line);
    _NObs=std::stoi(line);
    if(_NObs)
    {
        //line 6
        _iObs.reset(new int[_NObs]);
        std::getline(in,line);
        QString s=QString(line.c_str()).simplified();
        QStringList sl=s.split(' ');
        bool bok;
        for(int i=0;i<_NObs;++i)
        {
            _iObs[i]=sl[i].toInt(&bok);
            if(!bok)
            {
                return false;
            }
        }
    }
    return true;
}

bool ProfileObject::open(int gid, QSqlQuery &qry)
{
    _NumNP=_sel->_NumNP;
    _n.reset(new int[_NumNP]);
    _x.reset(new double[_NumNP]);
    _hNew.reset(new double[_NumNP]);
    _MatNum.reset(new int[_NumNP]);
    _LayNum.reset(new int[_NumNP]);
    _Beta.reset(new double[_NumNP]);
    _Ah.reset(new double[_NumNP]);
    _Ak.reset(new double[_NumNP]);
    _Ath.reset(new double[_NumNP]);
    _NS=_sel->NS;
    if(_NS)
    {
        _Conc.reset(new double[_NumNP*_NS]);
        if(!_sel->lEquil)
        {
            _Sorb.reset(new double[_NumNP*_NS]);
        }
    }
    _NObs=_sel->_NObs;
    if(_NObs)
    {
        _iObs.reset(new int[_NObs]);
        for(int i=0;i<_NObs;++i)
        {
            _iObs[i]=_sel->_iObs[i];
        }
    }
    std::stringstream strbld;
    strbld<<"select n, x, hnew, matnum, laynum,beta,ah,ak,ath";
    if(_NS)
    {
        strbld<<",";
        for(int i=1;i<_NS;++i)
        {
            strbld<<"conc"<<i<<",";
        }
        if(_sel->lEquil)
        {
            strbld<<"conc"<<_NS<<" from profile where gid="<<gid<<" order by n;";
        }
        else
        {
            strbld<<"conc"<<_NS<<",";
            for(int i=1;i<_NS;++i)
            {
                strbld<<"sorb"<<i<<",";
            }
            strbld<<"sorb"<<_NS<<" from profile where gid="<<gid<<" order by n;";
        }
        if(!qry.exec(strbld.str().c_str()))
        {
            return false;
        }
        int j=0;
        while (qry.next())
        {
            _n[j]=qry.value(0).toInt();
            _x[j]=qry.value(1).toDouble();
            _hNew[j]=qry.value(2).toDouble();
            _MatNum[j]=qry.value(3).toInt();
            _LayNum[j]=qry.value(4).toInt();
            _Beta[j]=qry.value(5).toDouble();
            _Ah[j]=qry.value(6).toDouble();
            _Ak[j]=qry.value(7).toDouble();
            _Ath[j]=qry.value(8).toDouble();
            double* pp=_Conc.get()+j*_NS;
            for(int i=0;i<_NS;++i)
            {
                pp[i]=qry.value(9+i).toDouble();
            }
            if(!_sel->lEquil)
            {
                pp=_Sorb.get()+j*_NS;
                for(int i=0;i<_NS;++i)
                {
                    pp[i]=qry.value(9+_NS+i).toDouble();
                }
            }
            j++;
        }
    }
    else
    {
        strbld<<" from profile where gid="<<gid<<" order by n;";
        if(!qry.exec(strbld.str().c_str()))
        {
            return false;
        }
        int j=0;
        while (qry.next())
        {
            _n[j]=qry.value(0).toInt();
            _x[j]=qry.value(1).toDouble();
            _hNew[j]=qry.value(2).toDouble();
            _MatNum[j]=qry.value(3).toInt();
            _LayNum[j]=qry.value(4).toInt();
            _Beta[j]=qry.value(5).toDouble();
            _Ah[j]=qry.value(6).toDouble();
            _Ak[j]=qry.value(7).toDouble();
            _Ath[j]=qry.value(8).toDouble();
            j++;
        }
    }
    return true;
}

bool ProfileObject::ParseLine(const std::string &line, const std::string &lineformat, const std::vector<void *> &values)
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

bool ProfileObject::ParseLine(const std::string &line, const int lineindex)
{
    QString s=QString(line.c_str()).simplified();
    QStringList sl=s.split(' ');
    bool bok;
    _n[lineindex]=sl[0].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    _x[lineindex]=sl[1].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _hNew[lineindex]=sl[2].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _MatNum[lineindex]=sl[3].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    _LayNum[lineindex]=sl[4].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    _Beta[lineindex]=sl[5].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ah[lineindex]=sl[6].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ak[lineindex]=sl[7].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ath[lineindex]=sl[8].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    return true;
}

bool ProfileObject::ParseLine(const std::string &line, const int lineindex, const int NS)
{
    QString s=QString(line.c_str()).simplified();
    QStringList sl=s.split(' ');
    bool bok;
    _n[lineindex]=sl[0].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    _x[lineindex]=sl[1].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _hNew[lineindex]=sl[2].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _MatNum[lineindex]=sl[3].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    _LayNum[lineindex]=sl[4].toInt(&bok);
    if(!bok)
    {
        return false;
    }
    _Beta[lineindex]=sl[5].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ah[lineindex]=sl[6].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ak[lineindex]=sl[7].toDouble(&bok);
    if(!bok)
    {
        return false;
    }
    _Ath[lineindex]=sl[8].toDouble(&bok);
    if(!bok)
    {
        return false;
    }

    double *pp=_Conc.get()+lineindex*_NS;
    for(int i=0;i<_NS;++i)
    {
        pp[i]=sl[10+i].toDouble(&bok);
        if(!bok)
        {
            return false;
        }
    }
    if(!_sel->lEquil)
    {
        pp=_Sorb.get()+lineindex*_NS;
        for(int i=0;i<NS;++i)
        {
            pp[i]=sl[10+NS+i].toDouble(&bok);
            if(!bok)
            {
                return false;
            }
        }
    }
    return true;
}

void ProfileObject::Initial()
{

}

void ProfileObject::SaveLine(std::ostream &out, const int lineindex)
{
    auto precision=out.precision();
    out<<std::setw(4)<<_n[lineindex];
    out<<std::fixed<<std::setprecision(6);
    out<<std::setw(15)<<fwzformat::SE3<<_x[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_hNew[lineindex];
    out.unsetf(std::ios_base::fixed);
    out<<std::setw(4)<<_MatNum[lineindex];
    out<<std::setw(4)<<_LayNum[lineindex];
    out<<std::fixed<<std::setprecision(6);
    out<<std::setw(15)<<fwzformat::SE3<<_Beta[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_Ah[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_Ak[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_Ath[lineindex];
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    out<<std::endl;
}

void ProfileObject::SaveLine(std::ostream &out,const int lineindex,const int NS)
{
    auto precision=out.precision();
    out<<std::setw(4)<<_n[lineindex];
    out<<std::fixed<<std::setprecision(6);
    out<<std::setw(15)<<fwzformat::SE3<<_x[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_hNew[lineindex];
    out.unsetf(std::ios_base::fixed);
    out<<std::setw(4)<<_MatNum[lineindex];
    out<<std::setw(4)<<_LayNum[lineindex];
    out<<std::fixed<<std::setprecision(6);
    out<<std::setw(15)<<fwzformat::SE3<<_Beta[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_Ah[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_Ak[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<_Ath[lineindex];
    out<<std::setw(15)<<fwzformat::SE3<<20.0;
    double *pp=_Conc.get()+lineindex*NS;
    for(int i=0;i<NS;++i)
    {
        out<<std::setw(15)<<fwzformat::SE3<<pp[i];
    }
    if(!_sel->lEquil)
    {
        pp=_Sorb.get()+lineindex*NS;
        for(int i=0;i<NS;++i)
        {
            out<<std::setw(15)<<fwzformat::SE3<<pp[i];
        }
    }
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    out<<std::endl;
}

