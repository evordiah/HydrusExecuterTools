
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

#include <QDir>
#include <fstream>
#include <QString>
#include <sstream>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <map>
#include <regex>
#include "HydrusParameterFilesManager.h"

HydrusParameterFilesManager::HydrusParameterFilesManager(int gid, const std::string &path, QSqlQuery &qry)
    :HydrusParameterFilesManager()
{
    _gid=gid;
    _path=path;
    _qry=&qry;
    _qry->exec("set constraint_exclusion = on;");
}

bool HydrusParameterFilesManager::ImportInputFiles()
{
    OpenInputFiles();
    if(!_isValid)
    {
        return false;
    }
    std::stringstream strbld;
    //strbld<<"set constraint_exclusion = on;";
    strbld<<"BEGIN TRANSACTION;";
    strbld<<_sel->ToSqlStatement(_gid);
    strbld<<_atm->ToSqlStatement(_gid);
    strbld<<_pro->ToSqlStatement(_gid);
    strbld<<"COMMIT;";
    return _qry->exec(strbld.str().c_str());
}

bool HydrusParameterFilesManager::ExportInputFiles()
{
    if(!_sel)
    {
        _sel.reset(new SelectorObject(_gid,*_qry,this));
        _isValid=*_sel;
        if(!_isValid)
        {
            _sel.reset();
            return false;
        }
        _isInitial=true;
    }
    if(!_atm && _sel)
    {
        _atm.reset(new AtmosphObject(_gid,*_qry,_sel.get()));
        _isValid=*_atm;
        if(!_isValid)
        {
            _atm.reset();
            return false;
        }
    }
    if(!_pro && _sel )
    {
        _pro.reset(new ProfileObject(_gid,*_qry,_sel.get()));
        _isValid=*_pro;
        if(!_isValid)
        {
            _pro.reset();
            return false;
        }
    }

    if(_sel->Save(_path) && _atm->Save(_path) && _pro->Save(_path))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool HydrusParameterFilesManager::ImportResultFiles()
{
    std::unique_ptr<std::string> _sptr=GetImportResultFilesSQlStatement();
    return _qry->exec(_sptr->c_str());
}

bool HydrusParameterFilesManager::ExportResultFiles()
{
    QString sql=QString("select lunit,tunit,munit,ns,nobs,iobs,"
                        "iday,imonth,ihours,imins,isecs,nlay,caltm "
                        "from selector where status='done' and gid=%1 ;").arg(_gid);
    if(_qry->exec(sql) && _qry->next())
    {
        _LUnit=_qry->value(0).toString().toStdString();
        _TUnit=_qry->value(1).toString().toStdString();
        _MUnit=_qry->value(2).toString().toStdString();
        _NS=_qry->value(3).toInt();
        _NObs=_qry->value(4).toInt();
        if(_NObs)
        {
            _iobs.reset(new int[_NObs]);
            _isValid=ParseSqlARRAY(_qry->value(5).toString().toStdString(),_iobs.get(),_NObs);
        }
        _iday=_qry->value(6).toInt();
        _imonth=_qry->value(7).toInt();
        _ihours=_qry->value(8).toInt();
        _imins=_qry->value(9).toInt();
        _isecs=_qry->value(10).toInt();
        _NLayer=_qry->value(11).toInt();
        _CalTm=_qry->value(12).toDouble();
        _isInitial=true;
    }
    else
    {
        _isValid=false;
        return false;
    }
    if(!_tlev)
    {
        _tlev.reset(new TLevelObject(_gid,*_qry,this));
        _isValid=*_tlev;
        if(!_isValid)
        {
            _tlev.reset();
            return false;
        }
    }
    if(!_alev)
    {
        _alev.reset(new ALevelObject(_gid,*_qry,this));
        _isValid=*_alev;
        if(!_isValid)
        {
            _alev.reset();
            return false;
        }
    }
    if(!_nod)
    {
        _nod.reset(new NodInfoObject(_gid,*_qry,this));
        _isValid=*_nod;
        if(!_isValid)
        {
            _nod.reset();
            return false;
        }
    }
    if(!_bal)
    {
        _bal.reset(new BalanceObject(_gid,*_qry,this));
        _isValid=*_bal;
        if(!_isValid)
        {
            _bal.reset();
            return false;
        }
    }
    if(!_obs  && _NObs)
    {
        _obs.reset(new ObsNodeObject(_gid,*_qry,this));
        _isValid=*_obs;
        if(!_isValid)
        {
            _obs.reset();
            return false;
        }
    }
    for(int i=1;i<=_NS;++i)
    {
        if(!_sol[i-1])
        {
            _sol[i-1].reset(new SoluteObject(_gid,*_qry,this,i));
            _isValid=*_sol[i-1];
            if(!_isValid)
            {
                _sol[i-1].reset();
                return false;
            }
        }
    }

    if(!_tlev->Save(_path))
    {
        return false;
    }
    if(!_alev->Save(_path))
    {
        return false;
    }
    if(!_nod->Save(_path))
    {
        return false;
    }
    if(!_bal->Save(_path))
    {
        return false;
    }
    if(_NObs && _obs && !_obs->Save(_path))
    {
        return false;
    }
    for(int i=1;i<=_NS;++i)
    {
        if(_sol[i-1] && !_sol[i-1]->Save(_path))
        {
            return false;
        }
    }
    return true;
}

bool HydrusParameterFilesManager::DropResultFiles()
{
    if(!_isValid)
    {
        return false;
    }
    QString s=QString("select * from clearhydrusresultbyid(%1);").arg(_gid);
    return _qry->exec(s);
}

bool HydrusParameterFilesManager::DropCase()
{
    if(!_isValid)
    {
        return false;
    }
    QString s=QString("select * from removehydruscasebyid(%1)").arg(_gid);
    return _qry->exec(s);
}

bool HydrusParameterFilesManager::DropInputFiles()
{
    if(!_isValid)
    {
        return false;
    }
    QString s=QString("select * from removehydrusinputparambyid(%1);").arg(_gid);
    return _qry->exec(s);
}

std::unique_ptr<std::string> HydrusParameterFilesManager::GetImportResultFilesSQlStatement()
{
    std::unique_ptr<std::string> _sptr;
    if(!_isInitial)
    {
        return _sptr;
    }
    OpenResultFiles();
    if(!_isValid)
    {
        return _sptr;
    }
    std::stringstream strbld;
    if(_err)
    {
        strbld<<_errMessage;
    }
    else
    {
        //strbld<<"set constraint_exclusion = on;";
        strbld<<"BEGIN TRANSACTION;";
        if(_status=="done")
        {
            strbld<<"select * from clearhydrusresultbyid("<<_gid<<");";
        }
        strbld<<_tlev->ToSqlStatement(_gid);
        strbld<<_alev->ToSqlStatement(_gid);
        strbld<<_nod->ToSqlStatement(_gid);
        strbld<<_bal->ToSqlStatement(_gid);
        if(_NObs)
        {
            strbld<<_obs->ToSqlStatement(_gid);
        }
        for(int i=0;i<_NS;++i)
        {
            strbld<<_sol[i]->ToSqlStatement(_gid);
        }
        strbld<<"select * from updateselectorstatus("<<_gid<<",true);";
        strbld<<"COMMIT;";
    }
    _sptr.reset(new std::string(strbld.str()));
    return _sptr;
}

HydrusParameterFilesManager::HydrusParameterFilesManager()
{
    _err=false;
    _errMessage="";
    _Hed="Welcome to HYDRUS-1D (HydrusExecuter)";
    _NLayer=0;
    _NS=0;
    _HeadLine=5;
    _iday=0;
    _imonth=0;
    _ihours=0;
    _imins=0;
    _isecs=0;
    _NObs=0;
    _LUnit="cm";
    _TUnit="days";
    _MUnit="mmol";
    _CalTm=std::numeric_limits<double>::max();
    _qry=nullptr;
    _isValid=true;
    _isInitial=false;
}

bool HydrusParameterFilesManager::ParseSqlARRAY(const std::string &value, int *p, int nsize)
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

void HydrusParameterFilesManager::GetErrMessage(const std::string& filename)
{
    std::ifstream in(filename);
    if(!in)
    {
        _isValid=false;
        return;
    }
    in.seekg(0,std::ios::end);
    int pos=static_cast<int>(in.tellg());
    std::unique_ptr<char[]> pbuf(new char[pos+1]);
    in.seekg(0,std::ios::beg);
    in.read(pbuf.get(),pos);
    in.close();
    pbuf[pos]='\0';
    QString s(pbuf.get());
    s=s.replace('\'','\"');
    _errMessage=s.simplified().toStdString();
}

void HydrusParameterFilesManager::OpenInputFiles()
{
    std::map<std::string,std::string> dic;
    QDir p(_path.c_str());
    QStringList lst=p.entryList();

    for(auto it=lst.begin();it!=lst.end();++it)
    {
        if(!it->compare("selector.in",Qt::CaseInsensitive))
        {
            std::string f=p.absoluteFilePath(*it).toStdString();
            if(!_sel)
            {
                _sel.reset(new SelectorObject(f,this));
                _isValid=*_sel;
                if(!_isValid)
                {
                    _sel.reset();
                    return;
                }
            }
        }
        else if(!it->compare("atmosph.in",Qt::CaseInsensitive))
        {
            dic["atmosph"]=p.absoluteFilePath(*it).toStdString();
        }
        else if(!it->compare("profile.dat",Qt::CaseInsensitive))
        {
            dic["profile"]=p.absoluteFilePath(*it).toStdString();
        }
    }

    if(!_atm && _sel && dic.find("atmosph")!=dic.end() )
    {
        _atm.reset(new AtmosphObject(dic["atmosph"],_sel.get()));
        _isValid=*_atm;
        if(!_isValid)
        {
            _atm.reset();
            return;
        }
    }
    if(!_pro && _sel && dic.find("profile")!=dic.end())
    {
        _pro.reset(new ProfileObject(dic["profile"],_sel.get()));
        _isValid=*_pro;
        if(!_isValid)
        {
            _pro.reset();
            return;
        }
    }

    if(!_sel || !_atm || !_pro)
    {
        _isValid=false;
    }
    _isInitial=true;
}

void HydrusParameterFilesManager::OpenResultFiles()
{
    std::map<std::string,std::string> dic;
    QDir p(_path.c_str());
    QStringList lst=p.entryList();
    for(auto it=lst.begin();it!=lst.end();++it)
    {
        if(!it->compare("error.msg",Qt::CaseInsensitive))
        {
            _err=true;
            GetErrMessage(p.absoluteFilePath(*it).toStdString());
            return;
        }
        else if(!it->compare("t_level.out",Qt::CaseInsensitive))
        {
            std::string val=p.absoluteFilePath(*it).toStdString();

            if(!_tlev)
            {
                _tlev.reset(new TLevelObject(val,this));
                _isValid=*_tlev;
                if(!_isValid)
                {
                    _tlev.reset();
                    return;
                }
            }
        }
        else if(!it->compare("a_level.out",Qt::CaseInsensitive))
        {
            dic["alevel"]=p.absoluteFilePath(*it).toStdString();
        }
        else if(!it->compare("nod_inf.out",Qt::CaseInsensitive))
        {
            dic["nod_inf"]=p.absoluteFilePath(*it).toStdString();
        }
        else if(!it->compare("obs_node.out",Qt::CaseInsensitive))
        {
            dic["obs_node"]=p.absoluteFilePath(*it).toStdString();
        }
        else if(!it->compare("balance.out",Qt::CaseInsensitive))
        {
            dic["balance"]=p.absoluteFilePath(*it).toStdString();
        }
        else
        {
            for(int i=1;i<=10;++i)
            {
                QString s=QString("solute%1.out").arg(i);
                if(!it->compare(s,Qt::CaseInsensitive))
                {
                    dic[QString("solute%1").arg(i).toStdString()]=p.absoluteFilePath(*it).toStdString();
                    break;
                }
            }
        }
    }

    if(!_alev && dic.find("alevel")!=dic.end())
    {
        _alev.reset(new ALevelObject(dic["alevel"],this));
        _isValid=*_alev;
        if(!_isValid)
        {
            _alev.reset();
            return;
        }
    }
    if(!_nod && dic.find("nod_inf")!=dic.end())
    {
        _nod.reset(new NodInfoObject(dic["nod_inf"],this));
        _isValid=*_nod;
        if(!_isValid)
        {
            _nod.reset();
            return;
        }
    }
    if(!_bal && dic.find("balance")!=dic.end())
    {
        _bal.reset(new BalanceObject(dic["balance"],this));
        _isValid=*_bal;
        if(!_isValid)
        {
            _bal.reset();
            return;
        }
    }
    if(!_obs && dic.find("obs_node")!=dic.end())
    {
        _obs.reset(new ObsNodeObject(dic["obs_node"],this));
        _isValid=*_obs;
        if(!_isValid)
        {
            _obs.reset();
            return;
        }
    }
    for(int i=1;i<=10;++i)
    {
        std::string key=QString("solute%1").arg(i).toStdString();
        if(!_sol[i-1] && dic.find(key)!=dic.end())
        {
            _sol[i-1].reset(new SoluteObject(dic[key],this));
            _isValid=*_sol[i-1];
            if(!_isValid)
            {
                _sol[i-1].reset();
                return;
            }
        }
    }

    if(!_tlev || !_alev || !_nod || !_bal)
    {
        _isValid=false;
    }
}
