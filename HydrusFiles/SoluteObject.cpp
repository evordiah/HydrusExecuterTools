
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

#include <iomanip>
#include <cstring>
#include <fstream>
#include <QDir>
#include <QFileInfo>
#include <string>
#include <sstream>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include "HydrusParameterFilesManager.h"
#include "FFmt.h"
#include "SoluteObject.h"

SoluteObject::SoluteObject(const std::string &filename,HydrusParameterFilesManager * parent)
{
    _parent=parent;
    _NObs=_parent->NumofObsNodes();
    _NObs=_NObs>3?3:_NObs;
    _isValid=open(filename);
}

SoluteObject::SoluteObject(int gid, QSqlQuery &qry, HydrusParameterFilesManager *parent, const int index)
{
    _parent=parent;
    _FileIndex=index;
    _NObs=_parent->NumofObsNodes();
    _NObs=_NObs>3?3:_NObs;
    _isValid=open(gid,qry);
}


SoluteObject::~SoluteObject()
{
    //dtor
}

bool SoluteObject::Save(const std::string &path)
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
    QString Filename=QString("solute%1.out").arg(_FileIndex);
    p=dir.absoluteFilePath(Filename);
    std::ofstream out(p.toStdString());
    if(!out)
    {
        return false;
    }
    //    format(' All solute fluxes and cumulative solute fluxes are positi
    //        !ve into the region'//
    //        !'       Time         cvTop        cvBot      Sum(cvTop)   Sum(cvBo
    //        !t)     cvCh0        cvCh1         cTop        cRoot         cBot
    //        !      cvRoot    Sum(cvRoot)  Sum(cvNEql) TLevel      cGWL        c
    //        !RunOff   Sum(cRunOff)    (cv(i),    Sum(cv(i)), i=1,NObs)'/
    //        !'        [T]        [M/L2/T]     [M/L2/T]      [M/L2]       [M/L2]
    //        !       [M/L2]      [M/L2]        [M/L3]      [M/L3]        [M/L3]
    //        !     [M/L2/T]      [M/L2]       [M/L2]              [M/L3]
    //        ![M/L2]      [M/L3]      [M/L2/T]      [M/L2]')
    out<<" All solute fluxes and cumulative solute fluxes are positi"
         "ve into the region"<<std::endl<<std::endl;
    out<<"       Time         cvTop        cvBot      Sum(cvTop)   Sum(cvBo"
         "t)     cvCh0        cvCh1         cTop        cRoot         cBot  "
         "      cvRoot    Sum(cvRoot)  Sum(cvNEql) TLevel      cGWL        c"
         "RunOff   Sum(cRunOff)    (cv(i),    Sum(cv(i)), i=1,NObs)"<<std::endl;
    out<<"        [T]        [M/L2/T]     [M/L2/T]      [M/L2]       [M/L2]"
         "       [M/L2]      [M/L2]        [M/L3]      [M/L3]        [M/L3] "
         "     [M/L2/T]      [M/L2]       [M/L2]              [M/L3]        "
         "[M/L2]      [M/L3]      [M/L2/T]      [M/L2]"<<std::endl;
    for(auto it=_Recs.begin();it!=_Recs.end();++it)
    {
        SaveLine(out,**it);
    }
    out<<"end"<<std::endl;
    out.close();
    return true;
}

std::string SoluteObject::ToSqlStatement(const int gid)
{
    std::stringstream out;
    out<<"INSERT INTO solute"<<_FileIndex<<"(gid, "
         "tm, cvtop, cvbot, s_cvtop, s_cvbot, s_cvch0, s_cvch1, ctop, "
         "croot, cbot, cvroot, s_cvroot, s_cvneql, t_level, cgwl, crunoff,s_crunoff";
    for(int i=1;i<=_NObs;++i)
    {
        out<<",cv"<<i<<",s_cv"<<i;
    }
    out<<") values";
    for(auto it=_Recs.begin();it!=_Recs.end();++it)
    {
        out<<"("<<gid<<","
          <<ToSqlStatement(**it)<<"),";
    }
    std::string sql=out.str();
    sql.back()=';';
    return sql;
}

bool SoluteObject::open(const std::string &filename)
{
    QFileInfo fi(filename.c_str());
    QString baseName=fi.baseName();
    _FileIndex=baseName.mid(6).toInt();
    std::ifstream in(filename);
    if(!in)
    {
        return false;
    }
    //ignore the head lines
    int i=0;
    std::string line;
    while(i++<4)
    {
        std::getline(in,line);
    }
    std::unique_ptr<SoluteRecord> pRec;
    while(true)
    {
        std::getline(in,line);
        if(line.substr(0,3)=="end")
        {
            break;
        }
        else
        {
            pRec.reset(new SoluteRecord(line.c_str(),_NObs));
            _Recs.push_back(std::move(pRec));
        }
    }
    return true;
}

bool SoluteObject::open(int gid, QSqlQuery &qry)
{
    std::stringstream strbld;
    strbld<<"select tm, cvtop, cvbot, s_cvtop, s_cvbot, s_cvch0, s_cvch1, ctop, "
            "croot, cbot, cvroot, s_cvroot, s_cvneql, t_level, cgwl, crunoff,s_crunoff";
    for(int i=1;i<=_NObs;++i)
    {
        strbld<<",cv"<<i<<",s_cv"<<i;
    }
    strbld<<" from solute"<<_FileIndex<<" where gid="<<gid<<" order by tm;";
    if(!qry.exec(strbld.str().c_str()))
    {
        return false;
    }
    std::unique_ptr<SoluteRecord> pRec;
    while(qry.next())
    {
        pRec.reset(new SoluteRecord(qry,_NObs));
        _Recs.push_back(std::move(pRec));
    }
    return true;
}

//format(f14.4,12e13.5,i8,e13.5,8e13.5)
void SoluteObject::SaveLine(std::ostream &os, const SoluteObject::SoluteRecord &srec)
{
    os<<std::fixed<<std::setprecision(4);
    os<<std::setw(14)<<srec.Time;
    os<<std::setprecision(5);
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cvTop;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cvBot;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cvTop;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cvBot;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cvCh0;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cvCh1;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cTop;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cRoot;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cBot;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cvRoot;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cvRoot;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cvNEql;
    os<<std::setw(8)<<srec.TLevel;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cGWL;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.cRunOff;
    os<<std::setw(13)<<fwzformat::fortranE2<<srec.sum_cRunOff;
    for(int i=0;i<_NObs;++i)
    {
        os<<std::setw(13)<<fwzformat::fortranE2<<srec.cv[i];
        os<<std::setw(13)<<fwzformat::fortranE2<<srec.sumcv[i];
    }
    os<<std::endl;
}

std::string SoluteObject::ToSqlStatement(const SoluteObject::SoluteRecord &srec)
{
    std::stringstream strbld;
    strbld<<std::fixed<<std::setprecision(4);
    strbld<<srec.Time<<",";
    strbld<<std::setprecision(5);
    strbld<<fwzformat::fortranE2<<srec.cvTop<<",";
    strbld<<fwzformat::fortranE2<<srec.cvBot<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cvTop<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cvBot<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cvCh0<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cvCh1<<",";
    strbld<<fwzformat::fortranE2<<srec.cTop<<",";
    strbld<<fwzformat::fortranE2<<srec.cRoot<<",";
    strbld<<fwzformat::fortranE2<<srec.cBot<<",";
    strbld<<fwzformat::fortranE2<<srec.cvRoot<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cvRoot<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cvNEql<<",";
    strbld<<srec.TLevel<<",";
    strbld<<fwzformat::fortranE2<<srec.cGWL<<",";
    strbld<<fwzformat::fortranE2<<srec.cRunOff<<",";
    strbld<<fwzformat::fortranE2<<srec.sum_cRunOff;
    for(int i=0;i<_NObs;++i)
    {
        strbld<<","<<fwzformat::fortranE2<<srec.cv[i];
        strbld<<","<<fwzformat::fortranE2<<srec.sumcv[i];
    }
    return strbld.str();
}

/*-------------Hydrus OUTPUT.FOR output format for each line in solute.out----------
format(f14.4,12e13.5,i8,e13.5,8e13.5)
-----------------------------------------------------------------------------------*/

SoluteObject::SoluteRecord::SoluteRecord(const char *pline, const int NOBS)
{
    int index[23]=
    {
        14,13,13,13,13,
        13,13,13,13,13,
        13,13,13,8,13,
        13,13,13,13,13,
        13,13,13
    };
    char split[23][15]= {0};
    char* psrc=const_cast<char*>(pline);
    for(int i=0; i<17; i++)
    {
        std::memcpy(&split[i][0],psrc,index[i]);
        psrc+=index[i];
    }
    for(int i=0;i<NOBS*2;++i)
    {
        std::memcpy(&split[17+i][0],psrc,index[17+i]);
        psrc+=index[17+i];
    }
    Time=atof(split[0]);
    cvTop=atof(split[1]);
    cvBot=atof(split[2]);
    sum_cvTop=atof(split[3]);
    sum_cvBot=atof(split[4]);
    sum_cvCh0=atof(split[5]);
    sum_cvCh1=atof(split[6]);
    cTop=atof(split[7]);
    cRoot=atof(split[8]);
    cBot=atof(split[9]);
    cvRoot=atof(split[10]);
    sum_cvRoot=atof(split[11]);
    sum_cvNEql=atof(split[12]);
    TLevel=atoi(split[13]);
    cGWL=atof(split[14]);
    cRunOff=atof(split[15]);
    sum_cRunOff=atof(split[16]);
    for(int i=0;i<NOBS;++i)
    {
        cv[i]=atof(split[17+i*2]);
        sumcv[i]=atof(split[18+i*2]);
    }
}

SoluteObject::SoluteRecord::SoluteRecord(QSqlQuery &qry, const int NOBS)
{
    Time=qry.value(0).toDouble();
    cvTop=qry.value(1).toDouble();
    cvBot=qry.value(2).toDouble();
    sum_cvTop=qry.value(3).toDouble();
    sum_cvBot=qry.value(4).toDouble();
    sum_cvCh0=qry.value(5).toDouble();
    sum_cvCh1=qry.value(6).toDouble();
    cTop=qry.value(7).toDouble();
    cRoot=qry.value(8).toDouble();
    cBot=qry.value(9).toDouble();
    cvRoot=qry.value(10).toDouble();
    sum_cvRoot=qry.value(11).toDouble();
    sum_cvNEql=qry.value(12).toDouble();
    TLevel=qry.value(13).toInt();
    cGWL=qry.value(14).toDouble();
    cRunOff=qry.value(15).toDouble();
    sum_cRunOff=qry.value(16).toDouble();
    for(int i=0;i<NOBS;++i)
    {
        cv[i]=qry.value(17+i*2).toDouble();
        sumcv[i]=qry.value(18+i*2).toDouble();
    }
}
