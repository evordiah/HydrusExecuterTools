
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
#include <sstream>
#include <string>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include "HydrusParameterFilesManager.h"
#include "NodInfoObject.h"
#include "FFmt.h"

NodInfoObject::NodInfoObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent=parent;
    _isValid=open(filename);
}

NodInfoObject::NodInfoObject(int gid, QSqlQuery &qry, HydrusParameterFilesManager *parent)
{
    _parent=parent;
    _isValid=open(gid,qry);
}


NodInfoObject::~NodInfoObject()
{
    //dtor
}

bool NodInfoObject::Save(const std::string &path)
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
    p=dir.absoluteFilePath("Nod_Inf.out");
    std::ofstream out(p.toStdString());
    if(!out)
    {
        return false;
    }
    WriteHead(out);
    for(auto it=_Recs.begin();it!=_Recs.end();++it)
    {
        WriteSection(out,it->first,it->second,_parent->NumofSolute());
    }
    out.close();
    return true;
}

std::string NodInfoObject::ToSqlStatement(const int gid)
{
    std::stringstream out;
    out<<"INSERT INTO nod_info("
         "gid, tm, node, depth, head, "
         "moistrue, k, c, flux, sink, "
         "kappa, vdivkstop,temp";
    int NS=_parent->NumofSolute();
    if(NS)
    {
        out<<",";
        for(int i=1;i<=NS;++i)
        {
            out<<"conc"<<i<<",";
        }
        for(int i=1;i<NS;++i)
        {
            out<<"sorb"<<i<<",";
        }
        out<<"sorb"<<NS
          <<") VALUES";
    }
    else
    {
        out<<") VALUES";
    }
    for(auto it=_Recs.begin();it!=_Recs.end();++it)
    {
        for(auto jt=it->second.begin();jt!=it->second.end();++jt)
        {
            out<<"("<<gid<<","<<it->first<<","
              <<ToSqlStatement(**jt,NS)
             <<"),";
        }
    }
    std::string sql=out.str();
    sql.back()=';';
    return sql;
}

bool NodInfoObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if(!in)
    {
        return false;
    }
    
    int i=0;
    int NS=_parent->NumofSolute();
    //ignore the head lines
    std::string line;
    while(i++<_parent->HeadLineCount())
    {
        std::getline(in,line);
    }
    double tm;
    char strtm[15]={0};
    std::unique_ptr<NodinfoRecord> pRec;
    while(std::getline(in,line))
    {
        for(int j=0;j<7;++j)
        {
            std::getline(in,line);
            if(line.find(" Time:")!=std::string::npos)
            {
                std::memcpy(strtm,line.c_str()+6,14);
                strtm[14]='\0';
                tm=atof(strtm);
            }
        }
        while(std::getline(in,line))
        {
            if(line.substr(0,3)=="end")
            {
                break;
            }
            else
            {
                pRec.reset(new NodinfoRecord(line.c_str(),NS));
                _Recs[tm].push_back(std::move(pRec));

            }
        }
    }
    return true;
}

bool NodInfoObject::open(int gid, QSqlQuery &qry)
{
    std::stringstream strbld;
    strbld<<"select tm,node,depth,head,moistrue,"
            "k,c,flux,sink,kappa,vdivkstop,temp";
    int NS=_parent->NumofSolute();
    if(NS)
    {   strbld<<",";
        for(int i=1;i<=NS;++i)
        {
            strbld<<"conc"<<i<<",";
        }
        for(int i=1;i<NS;++i)
        {
            strbld<<"sorb"<<i<<",";
        }
        strbld<<"sorb"<<NS
             <<" from nod_info where gid="<<gid<<" order by tm,node;";
    }
    else
    {
        strbld<<" from nod_info where gid="<<gid<<" order by tm,node;";
    }
    if(!qry.exec(strbld.str().c_str()))
    {
        return false;
    }
    std::unique_ptr<NodinfoRecord> pRec;
    while(qry.next())
    {
        double tm=qry.value(0).toDouble();
        pRec.reset(new NodinfoRecord(qry,NS));
        _Recs[tm].push_back(std::move(pRec));
    }
    return true;
}

void NodInfoObject::WriteHead(std::ostream &out)
{
    out<<" ******* Program HYDRUS"<<std::endl;
    out<<std::left;
    if(_parent->HeadLineCount()==4)
    {
        out<<" ******* "<<std::setw(72)<<_parent->HeadContent()<<std::endl;
    }
    else
    {
        out<<" ******* "<<std::endl
          <<' '<<std::setw(72)<<_parent->HeadContent()<<std::endl;
    }
    out<<std::right;
    //Hydrus output head format
    //format(' Date: ',i3,'.',i2,'.','    Time: ',i3,':',i2,':',i2)
    out<<" Date: "<<std::setw(3)<<_parent->Day()<<'.'
      <<std::setw(2)<<_parent->Mon()<<'.'
     <<"    Time: "<<std::setw(3)<<_parent->Hour()<<':'
    <<std::setw(2)<<_parent->Mints()<<':'
    <<std::setw(2)<<_parent->Secs()<<std::endl;
    out<<std::left;
    out<<" Units: L = "<<std::setw(5)<<_parent->LUnit()
      <<", T = "<<std::setw(5)<<_parent->TUnit()
     <<", M = "<<std::setw(5)<<_parent->MUnit()<<std::endl;
    out<<std::right;
}
/*-------------Hydrus OUTPUT.FOR output format for Description------------
  format(//' Time:',f14.4//)
  format(
     !' Node      Depth      Head Moisture       K          C         ',
     !'Flux        Sink         Kappa   v/KsTop   Temp'/
     !'           [L]        [L]    [-]        [L/T]      [1/L]      [',
     !'L/T]        [1/T]         [-]      [-]      [C]'/)
-----------------------------------------------------------------------*/
void NodInfoObject::WriteSection(std::ostream &out, double time, std::list<std::unique_ptr<NodInfoObject::NodinfoRecord> > &lst, const int NS)
{
    out<<std::endl<<std::endl;
    out<<" Time:"<<std::fixed<<std::setw(14)<<std::setprecision(4)<<time<<std::endl;
    out<<std::endl<<std::endl;
    if(NS==0)
    {
        out<<" Node      Depth      Head Moisture       K          C         "
          <<"Flux        Sink         Kappa   v/KsTop   Temp"<<std::endl
         <<"           [L]        [L]    [-]        [L/T]      [1/L]      ["
           "L/T]        [1/T]         [-]      [-]      [C]"<<std::endl;
    }
    else
    {
        out<<" Node      Depth      Head Moisture       K          C         "
             "Flux        Sink         Kappa   v/KsTop   Temp   Conc(1..NS) Sorb(1...NS)"<<std::endl;
        out<<"           [L]        [L]    [-]        [L/T]      [1/L]      ["
             "L/T]        [1/T]         [-]      [-]      [C]      [M/L*3]"<<std::endl;
    }
    out<<std::endl;
    for(auto it=lst.begin();it!=lst.end();++it)
    {
        SaveLine(out,**it,NS);
    }
    out<<"end"<<std::endl;
}

void NodInfoObject::SaveLine(std::ostream &os, const NodInfoObject::NodinfoRecord &nrec, const int NS)
{
    os<<std::fixed;
    os<<std::setw(4)<<nrec.Node;
    os<<std::setprecision(4);
    os<<std::setw(11)<<nrec.Depth;
    os<<std::setprecision(3);
    os<<std::setw(12)<<nrec.Head;
    os<<std::setprecision(4);
    os<<std::setw(7)<<nrec.Moisture;
    os<<std::setw(13)<<fwzformat::fortranE2<<nrec.K;
    os<<std::setw(12)<<fwzformat::fortranE2<<nrec.C;
    os<<std::setw(12)<<fwzformat::fortranE2<<nrec.Flux;
    os<<std::setw(12)<<fwzformat::fortranE2<<nrec.Sink;
    os<<std::setw(8)<<nrec.Kappa;
    os<<std::setprecision(3);
    os<<std::setw(12)<<fwzformat::fortranE2<<nrec.vdivKsTop;
    os<<std::setprecision(2);
    os<<std::setw(8)<<nrec.Temp;
    if(NS)
    {
        os<<std::setprecision(4);
        for(int i=0;i<NS;++i)
        {
            os<<std::setw(12)<<fwzformat::fortranE2<<nrec._Conc[i];
        }
        for(int i=0;i<NS;++i)
        {
            os<<std::setw(12)<<fwzformat::fortranE2<<nrec._Sorb[i];
        }
    }
    os<<std::endl;
}

std::string NodInfoObject::ToSqlStatement(const NodInfoObject::NodinfoRecord &nrect, const int NS)
{
    std::stringstream strbld;
    auto precision=strbld.precision();
    strbld<<nrect.Node<<","
         <<nrect.Depth<<",";
    strbld<<std::fixed<<std::setprecision(3)
         <<nrect.Head<<",";
    strbld.unsetf(std::ios_base::fixed);
    strbld.precision(precision);
    strbld<<nrect.Moisture<<","
         <<nrect.K<<","
        <<nrect.C<<","
       <<nrect.Flux<<","
      <<nrect.Sink<<","
     <<nrect.Kappa<<","
    <<nrect.vdivKsTop<<","
    <<nrect.Temp<<",";
    if(NS)
    {
        for(int i=0;i<NS;++i)
        {
            strbld<<nrect._Conc[i]<<",";
        }
        for(int i=0;i<NS;++i)
        {
            strbld<<nrect._Sorb[i]<<",";
        }
    }
    std::string result=strbld.str();
    result.pop_back();
    return result;
}

/*-------------Hydrus OUTPUT.FOR output format for each line in A_Level.out----------
//format(f12.5,5e14.6,3f11.3,i8)
-----------------------------------------------------------------------------------*/
//format(i4,1x,f10.4,1x,f11.3,1x,f6.4,1x,4e12.4,i8,1x,e11.3,f8.2,
//    !       30e12.4)
NodInfoObject::NodinfoRecord::NodinfoRecord(const char *pline, const int NS)
{
    int index[31]=
    {
        4,11,12,7,13,
        12,12,12,8,12,8,
        12,12,12,12,12,12,12,12,12,12,
        12,12,12,12,12,12,12,12,12,12
    };
    char split[31][14]= {0};
    char* psrc=const_cast<char*>(pline);
    for(int i=0; i<11; i++)
    {
        std::memcpy(&split[i][0],psrc,index[i]);
        psrc+=index[i];
    }
    if(NS)
    {
        for(int i=0;i<2*NS;++i)
        {
            std::memcpy(&split[11+i][0],psrc,index[11+i]);
            psrc+=index[11+i];
        }
    }
    Node=atoi(split[0]);
    Depth=atof(split[1]);
    Head=atof(split[2]);
    Moisture=atof(split[3]);
    K=atof(split[4]);
    C=atof(split[5]);
    Flux=atof(split[6]);
    Sink=atof(split[7]);
    Kappa=atoi(split[8]);
    vdivKsTop=atof(split[9]);
    Temp=atof(split[10]);
    if(NS)
    {
        _Conc.reset(new double[NS]);
        _Sorb.reset(new double[NS]);
        for(int i=0;i<NS;++i)
        {
            _Conc[i]=atof(split[11+i]);
        }
        for(int i=0;i<NS;++i)
        {
            _Sorb[i]=atof(split[11+NS+i]);
        }
    }
}

NodInfoObject::NodinfoRecord::NodinfoRecord(QSqlQuery &qry, const int NS)
{
    Node=qry.value(1).toInt();
    Depth=qry.value(2).toDouble();
    Head=qry.value(3).toDouble();
    Moisture=qry.value(4).toDouble();
    K=qry.value(5).toDouble();
    C=qry.value(6).toDouble();
    Flux=qry.value(7).toDouble();
    Sink=qry.value(8).toDouble();
    Kappa=qry.value(9).toInt();
    vdivKsTop=qry.value(10).toDouble();
    Temp=qry.value(11).toDouble();
    if(NS)
    {
        _Conc.reset(new double[NS]);
        _Sorb.reset(new double[NS]);
        for(int i=0;i<NS;++i)
        {
            _Conc[i]=qry.value(12+i).toDouble();
        }
        for(int i=0;i<NS;++i)
        {
            _Sorb[i]=qry.value(12+NS+i).toDouble();
        }
    }
}


