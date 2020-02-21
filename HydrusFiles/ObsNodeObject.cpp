
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
#include <string>
#include <sstream>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include "HydrusParameterFilesManager.h"
#include "ObsNodeObject.h"
#include "FFmt.h"

ObsNodeObject::ObsNodeObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent=parent;
    _NS=_parent->NumofSolute();
    _NObs=_parent->NumofObsNodes();
    if(_NObs)
    {
        _iobs.reset(new int[_NObs]);
        for(int i=0;i<_NObs;++i)
        {
            _iobs[i]=_parent->ObsNodeIndex(i);
        }
        _isValid=open(filename);
    }
    else
    {
        _isValid=false;
    }
}

ObsNodeObject::ObsNodeObject(int gid, QSqlQuery &qry, HydrusParameterFilesManager *parent)
{
    _parent=parent;
    _NS=_parent->NumofSolute();
    _NObs=_parent->NumofObsNodes();
    if(_NObs)
    {
        _iobs.reset(new int[_NObs]);
        for(int i=0;i<_NObs;++i)
        {
            _iobs[i]=_parent->ObsNodeIndex(i);
        }
        _isValid=open(gid,qry);
    }
    else
    {
        _isValid=false;
    }
}

ObsNodeObject::~ObsNodeObject()
{

}

bool ObsNodeObject::Save(const std::string &path)
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
    p=dir.absoluteFilePath("Obs_Node.out");
    std::ofstream out(p.toStdString());
    if(!out)
    {
        return false;
    }
    WriteHead(out);
    for(int i=0;i<_tmCnt;++i)
    {
        SaveLine(out,i);
    }
    out<<"end"<<std::endl;
    return true;
}

std::string ObsNodeObject::ToSqlStatement(const int gid)
{
    std::stringstream out;
    out<<"INSERT INTO obs_node("
         "gid, tm, obsid, h, theta, flux";
    if(_NS)
    {
        out<<",";
        for(int i=1;i<_NS;++i)
        {
            out<<"conc"<<i<<",";
        }
        out<<"conc"<<_NS
          <<") VALUES";
    }
    else
    {
        out<<") VALUES";
    }
    for(int i=0;i<_tmCnt;++i)
    {
        out<<LineToSql(gid,i);
    }
    std::string sql=out.str();
    sql.back()=';';
    return sql;
}

bool ObsNodeObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if(!in)
    {
        return false;
    }
    //ignore the head lines
    int i=0;
    std::string line;
    while(i++<_parent->HeadLineCount()+6)
    {
        std::getline(in,line);
    }
    _tmCnt=0;
    std::list<std::string> _lines;
    while(std::getline(in,line))
    {
        if(line.substr(0,3)=="end")
        {
            break;
        }
        _lines.push_back(line);
        _tmCnt++;
    }
    in.close();
    if(_tmCnt)
    {
        _time.reset(new double[_tmCnt]);
        _head.reset(new double[_tmCnt*_NObs]);
        _theta.reset(new double[_tmCnt*_NObs]);
        _flux.reset(new double[_tmCnt*_NObs]);
        if(_NS)
        {
            _conc.reset(new double[_tmCnt*_NObs*_NS]);
        }
    }
    else
    {
        return false;
    }
    i=0;
    for(auto it=_lines.begin();it!=_lines.end();++it)
    {
        if(!ParseLine(it->c_str(),it->size(),i++))
        {
            return false;
        }
    }
    return true;
}

bool ObsNodeObject::open(int gid, QSqlQuery &qry)
{
    std::stringstream strbld;
    QString sql=QString("select count(distinct tm) from obs_node where gid=%1").arg(gid);
    if(!qry.exec(sql) || !qry.next() )
    {
        return false;
    }
    _tmCnt=qry.value(0).toInt();
    if(_tmCnt)
    {
        _time.reset(new double[_tmCnt]);
        _head.reset(new double[_tmCnt*_NObs]);
        _theta.reset(new double[_tmCnt*_NObs]);
        _flux.reset(new double[_tmCnt*_NObs]);
        if(_NS)
        {
            _conc.reset(new double[_tmCnt*_NObs*_NS]);
        }
    }
    strbld<<"select tm, h, theta, flux,";
    if(_NS)
    {
        for(int i=1;i<=_NS;++i)
        {
            strbld<<"conc"<<i<<",";
        }
        strbld<<"obsid from obs_node where gid="<<gid<<" order by tm,obsid;";
    }
    else
    {
        strbld<<"obsid from obs_node where gid="<<gid<<" order by tm,obsid;";
    }
    if(!qry.exec(strbld.str().c_str()))
    {
        return false;
    }
    int i=0;
    int cnt=0;
    int idx=0,idx2=0;
    while(qry.next())
    {
        if(cnt==0)
        {
            idx=i*_NObs;
            idx2=i*_NObs*_NS;
            _time[i]=qry.value(0).toDouble();
            i++;
        }
        _head[idx+cnt]=qry.value(1).toDouble();
        _theta[idx+cnt]=qry.value(2).toDouble();
        _flux[idx+cnt]=qry.value(3).toDouble();
        int id=qry.value(4).toInt();

        int k=idx2+cnt*_NS+id;
        for(int j=0;j<_NS;++j)
        {
            _conc[k+j]=qry.value(4+j).toDouble();
        }
        cnt++;
        if(cnt==_NObs)
        {
            cnt=0;
        }
    }
    return true;
}

void ObsNodeObject::WriteHead(std::ostream &out)
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

    int cnt=std::min(10,_NObs);
    std::string Text3="Node(";
    std::string Text1="    h        theta    Flux   ";
    std::string Text2="   Conc     ";
    if(_NS)
    {
        //format (///16x,10(15x,a5,i3,')',18x))
        out<<std::endl<<std::endl<<std::endl;
        if(_NS<=6)
        {
            out<<std::setw(36)<<Text3<<std::setw(3)<<_iobs[0]<<")";
        }
        else
        {
            out<<std::setw(34)<<Text3<<std::setw(3)<<_iobs[0]<<")";
        }
        for(int i=1;i<cnt;++i)
        {
            out<<std::setw(18+(_NS-1)*11)<<' ';
            out<<std::setw(20)<<Text3<<std::setw(3)<<_iobs[i]<<")";
        }
        out<<std::endl;
        // format (/'         time     ',10(a29, a12,2x))
        out<<std::endl;
        if(_NS<=6)
        {
            out<<"         time     ";
        }
        else
        {
            out<<"       time     ";
        }
        for(int i=0;i<cnt;++i)
        {
            out<<std::setw(29)<<Text1;
            for(int j=0;j<_NS;++j)
            {
                out<<std::setw(12)<<Text2;
            }
            if(i<cnt-1)
            {
                out<<"  ";
            }
        }
        out<<std::endl;
    }
    else
    {
        //format (///16x,10(15x,a5,i3,')', 7x))
        out<<std::endl<<std::endl<<std::endl;
        out<<std::setw(36)<<Text3<<std::setw(3)<<_iobs[0]<<")";
        for(int i=1;i<cnt;++i)
        {
            out<<std::setw(7)<<' ';
            out<<std::setw(20)<<Text3<<std::setw(3)<<_iobs[i]<<")";
        }
        out<<std::endl;
        //format (/'         time     ',10(a29,     2x))
        out<<std::endl;
        out<<"         time     ";
        for(int i=0;i<cnt;++i)
        {
            out<<std::setw(29)<<Text1;
            if(i<cnt-1)
            {
                out<<"  ";
            }
        }
        out<<std::endl;
    }
}

bool ObsNodeObject::ParseLine(const char *p, const int nsize, const int lineindex)
{
    char tmp[17]= {0};
    char* psrc=const_cast<char*>(p);
    std::memcpy(&tmp[0],psrc,16);
    _time[lineindex]=std::atof(tmp);
    if(lineindex && _time[lineindex]==_time[lineindex-1])
    {
        _time[lineindex]+=1e-5;
    }
    psrc+=16;

    switch (_NS)
    {
    case 0:
        if(nsize<(33*_NObs+16-2))
        {
            return  false;
        }
        else
        {
            int idx=lineindex*_NObs;
            for(int i=0;i<_NObs;++i)
            {
                std::memcpy(&tmp[0],psrc,12);
                tmp[12]=0;
                _head[idx+i]=std::atof(tmp);
                psrc+=12;
                std::memcpy(&tmp[0],psrc,8);
                tmp[8]=0;
                _theta[idx+i]=std::atof(tmp);
                psrc+=8;
                std::memcpy(&tmp[0],psrc,11);
                tmp[11]=0;
                _flux[idx+i]=std::atof(tmp);
                psrc+=13;
            }
        }
        break;
    case 1:
    case 2:
    case 3:
        if(nsize<(16+(22+12*(_NS+1))*_NObs)-2)
        {
            return  false;
        }
        else
        {
            int idx=lineindex*_NObs;
            int idx2=lineindex*_NObs*_NS;
            for(int i=0;i<_NObs;++i)
            {
                std::memcpy(&tmp[0],psrc,12);
                tmp[12]=0;
                _head[idx+i]=std::atof(tmp);
                psrc+=12;
                std::memcpy(&tmp[0],psrc,8);
                tmp[8]=0;
                _theta[idx+i]=std::atof(tmp);
                psrc+=8;
                std::memcpy(&tmp[0],psrc,12);
                tmp[12]=0;
                _flux[idx+i]=std::atof(tmp);
                psrc+=12;
                int k=idx2+i*_NS;
                for(int j=0;j<_NS;++j)
                {
                    std::memcpy(&tmp[0],psrc,12);
                    _conc[k+j]=std::atof(tmp);
                    psrc+=12;
                }
                psrc+=2;
            }
        }
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        if(nsize<(16+(31+12*_NS)*_NObs-2))
        {
            return  false;
        }
        else
        {
            int idx=lineindex*_NObs;
            int idx2=lineindex*_NObs*_NS;
            for(int i=0;i<_NObs;++i)
            {
                std::memcpy(&tmp[0],psrc,12);
                tmp[12]=0;
                _head[idx+i]=std::atof(tmp);
                psrc+=12;
                std::memcpy(&tmp[0],psrc,8);
                tmp[8]=0;
                _theta[idx+i]=std::atof(tmp);
                psrc+=8;
                std::memcpy(&tmp[0],psrc,9);
                tmp[9]=0;
                _flux[idx+i]=std::atof(tmp);
                psrc+=9;
                int k=idx2+i*_NS;
                tmp[12]=0;
                for(int j=0;j<_NS;++j)
                {
                    std::memcpy(&tmp[0],psrc,12);
                    _conc[k+j]=std::atof(tmp);
                    psrc+=12;
                }
                psrc+=2;
            }
        }
        break;
    default:
        return false;
    }
    return true;
}

void ObsNodeObject::SaveLine(std::ostream &out, const int lineindex)
{
    int precision=out.precision();
    out<<std::setw(16)<<std::setprecision(4)<<std::fixed<<_time[lineindex];
    int idx=lineindex*_NObs;
    int idx2=lineindex*_NObs*_NS;
    switch (_NS)
    {
    case 0:
        for(int i=0;i<_NObs;++i)
        {
            out<<std::setw(12)<<std::setprecision(2)<<_head[idx+i];
            out<<std::setw(8)<<std::setprecision(4)<<_theta[idx+i];
            out<<std::setw(11)<<std::setprecision(3)<<fwzformat::fortranE2<<_flux[idx+i];
            if(i<_NObs-1)
            {
                out<<"  ";
            }
        }
        break;
    case 1:
    case 2:
    case 3:
        for(int i=0;i<_NObs;++i)
        {
            out<<std::setw(12)<<std::setprecision(2)<<_head[idx+i];
            out<<std::setw(8)<<std::setprecision(4)<<_theta[idx+i];
            out<<std::setw(12)<<std::setprecision(4)<<fwzformat::fortranE2<<_flux[idx+i];
            int k=idx2+i*_NS;
            for(int j=0;j<_NS;++j)
            {
                out<<std::setw(12)<<std::setprecision(4)<<fwzformat::fortranE2<<_conc[k+j];
            }
            if(i<_NObs-1)
            {
                out<<"  ";
            }
        }
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        for(int i=0;i<_NObs;++i)
        {
            out<<std::setw(12)<<std::setprecision(2)<<_head[idx+i];
            out<<std::setw(8)<<std::setprecision(4)<<_theta[idx+i];
            out<<std::setw(9)<<std::setprecision(3)<<_flux[idx+i];
            int k=idx2+i*_NS;
            for(int j=0;j<_NS;++j)
            {
                out<<std::setw(12)<<std::setprecision(4)<<fwzformat::fortranE2<<_conc[k+j];
            }
            if(i<_NObs-1)
            {
                out<<"  ";
            }
        }
        break;
    default:
        break;
    }
    out<<std::endl;
    out.precision(precision);
    out.unsetf(std::ios_base::fixed);
}

std::string ObsNodeObject::LineToSql(const int gid, const int lineindex)
{
    int idx=lineindex*_NObs;
    int idx2=lineindex*_NObs*_NS;
    std::stringstream strbld;
    double tm=_time[lineindex];
    strbld<<std::fixed;
    for(int i=0;i<_NObs;++i)
    {
        strbld<<"("<<gid<<","
             <<std::setprecision(5)<<tm<<","
            <<_iobs[i]<<","
           <<std::setprecision(2)<<_head[idx+i]<<","
          <<std::setprecision(4)<<_theta[idx+i]<<","
         <<std::setprecision(4)<<fwzformat::fortranE2<<_flux[idx+i];
        int k=idx2+i*_NS;
        for(int j=0;j<_NS;++j)
        {
            strbld<<","<<std::setprecision(4)<<fwzformat::fortranE2<<_conc[k+j];
        }
        strbld<<"),";
    }

    return strbld.str();
}
