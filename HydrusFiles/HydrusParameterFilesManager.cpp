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

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <regex>
#include "tinyxml2/tinyxml2.h"
#include "HydrusParameterFilesManager.h"
#include "Stringhelper.h"

HydrusParameterFilesManager::HydrusParameterFilesManager(int gid, pqxx::connection &qry)
    : HydrusParameterFilesManager()
{
    _gid = gid;
    _path = "";
    _qry = &qry;
    try
    {
        pqxx::nontransaction nw(qry);
        nw.exec0("set constraint_exclusion = on;");
    }
    catch (...)
    {
    }
}

HydrusParameterFilesManager::HydrusParameterFilesManager(int gid, const std::string &path, pqxx::connection &qry)
    : HydrusParameterFilesManager()
{
    _gid = gid;
    _path = path;
    _qry = &qry;
    try
    {
        pqxx::nontransaction nw(qry);
        nw.exec0("set constraint_exclusion = on;");
    }
    catch (...)
    {
    }
}

HydrusParameterFilesManager::HydrusParameterFilesManager(int gid, const std::string &path,
                                                         pqxx::connection &qry, unsigned int nlayer,
                                                         unsigned int ns, unsigned int nobs,
                                                         const std::string &iobs, const std::string &status)
    : HydrusParameterFilesManager(nlayer, ns, nobs, nullptr, status)
{
    _gid = gid;
    _path = path;
    _qry = &qry;

    if (_NObs && !iobs.empty())
    {
        std::stringstream instream(iobs);
        for (int i = 0; i < _NObs; ++i)
        {
            instream >> _iobs[i];
        }
    }
    try
    {
        pqxx::nontransaction nw(qry);
        nw.exec0("set constraint_exclusion = on;");
    }
    catch (...)
    {
    }
    _isInitial = true;
}

HydrusParameterFilesManager::HydrusParameterFilesManager(int gid, const std::string &path, unsigned int nlayer,
                                                         unsigned int ns, unsigned int nobs, unsigned int *iobs,
                                                         const std::string &status)
    : HydrusParameterFilesManager(nlayer, ns, nobs, iobs, status)
{
    _gid = gid;
    _path = path;
    _qry = nullptr;
    _isInitial = true;
}

bool HydrusParameterFilesManager::ImportInputFiles()
{
    OpenInputFiles();
    if (!_isValid)
    {
        return false;
    }
    std::stringstream strbld;
    //strbld<<"set constraint_exclusion = on;";
    strbld << "BEGIN TRANSACTION;";
    strbld << _sel->ToSqlStatement(_gid);
    strbld << _atm->ToSqlStatement(_gid);
    strbld << _pro->ToSqlStatement(_gid);
    strbld << "COMMIT;";
    try
    {
        pqxx::work w(*_qry);
        w.exec(strbld.str());
        w.commit();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool HydrusParameterFilesManager::ExportInputFiles()
{
    if (_path.empty())
    {
        return false;
    }
    if (!_sel)
    {
        _sel = std::make_unique<SelectorObject>(_gid, *_qry, this);
        _isValid = *_sel;
        if (!_isValid)
        {
            _sel.reset();
            return false;
        }
        _isInitial = true;
    }

    if (!_atm && _sel)
    {
        _atm = std::make_unique<AtmosphObject>(_gid, *_qry, _sel.get());
        _isValid = *_atm;
        if (!_isValid)
        {
            _atm.reset();
            return false;
        }
    }

    if (!_pro && _sel)
    {
        _pro = std::make_unique<ProfileObject>(_gid, *_qry, _sel.get());
        _isValid = *_pro;
        if (!_isValid)
        {
            _pro.reset();
            return false;
        }
    }
    return _sel->Save(_path) && _atm->Save(_path) && _pro->Save(_path);
}

bool HydrusParameterFilesManager::ExportInputFiles(std::string &content)
{
    if (!_sel)
    {
        _sel = std::make_unique<SelectorObject>(_gid, *_qry, this);
        _isValid = *_sel;
        if (!_isValid)
        {
            _sel.reset();
            return false;
        }
        _isInitial = true;
    }
    if (!_atm && _sel)
    {
        _atm = std::make_unique<AtmosphObject>(_gid, *_qry, _sel.get());
        _isValid = *_atm;
        if (!_isValid)
        {
            _atm.reset();
            return false;
        }
    }
    if (!_pro && _sel)
    {
        _pro = std::make_unique<ProfileObject>(_gid, *_qry, _sel.get());
        _isValid = *_pro;
        if (!_isValid)
        {
            _pro.reset();
            return false;
        }
    }
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *mele = doc.NewElement("message");
    tinyxml2::XMLElement *cele = mele->InsertNewChildElement("content");
    cele->SetAttribute("success", true);
    cele->SetAttribute("status", _status.c_str());
    cele->SetAttribute("layer", _NLayer);
    cele->SetAttribute("ns", _NS);
    cele->SetAttribute("nobs", _NObs);
    std::stringstream sbuffer;
    if (_NObs)
    {
        sbuffer << _iobs[0];
        for (int i = 1; i < _NObs; ++i)
        {
            sbuffer << " " << _iobs[i];
        }
        std::string strobs = sbuffer.str();
        sbuffer.clear();
        cele->SetAttribute("obsnode", strobs.c_str());
    }

    sbuffer.str("");
    if (_sel->Save(sbuffer))
    {
        tinyxml2::XMLElement *sele = mele->InsertNewChildElement("selector");
        sele->SetAttribute("value", sbuffer.str().c_str());
    }
    else
    {
        return false;
    }
    sbuffer.str("");
    if (_atm->Save(sbuffer))
    {
        tinyxml2::XMLElement *aele = mele->InsertNewChildElement("atmosph");
        aele->SetAttribute("value", sbuffer.str().c_str());
    }
    else
    {
        return false;
    }
    sbuffer.str("");
    if (_pro->Save(sbuffer))
    {
        tinyxml2::XMLElement *pele = mele->InsertNewChildElement("profile");
        pele->SetAttribute("value", sbuffer.str().c_str());
    }
    else
    {
        return false;
    }
    doc.InsertFirstChild(mele);
    tinyxml2::XMLPrinter prt(0, true);
    doc.Accept(&prt);
    content.clear();
    content.append(prt.CStr());
    return true;
}

bool HydrusParameterFilesManager::ImportResultFiles()
{
    std::unique_ptr<std::string> _usptr = GetImportResultFilesSQlStatement();
    try
    {
        pqxx::work w(*_qry);
        w.exec(*_usptr);
        w.commit();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool HydrusParameterFilesManager::ExportResultFiles()
{
    std::string sql("select lunit,tunit,munit,ns,nobs,iobs,"
                    "iday,imonth,ihours,imins,isecs,nlay,caltm "
                    "from selector where status='done' and gid=$1 ;");
    try
    {
        pqxx::work w(*_qry);
        pqxx::row r = w.exec_params1(sql, _gid);
        w.commit();
        _LUnit = r[0].as<std::string>();
        _TUnit = r[1].as<std::string>();
        _MUnit = r[2].as<std::string>();
        r[3].to(_NS, 0);
        _NObs = r[4].as<int>();
        if (_NObs)
        {
            _iobs = std::make_unique<unsigned int[]>(_NObs);
            _isValid = ParseSqlARRAY(r[5].as<std::string>(), _iobs.get(), _NObs);
        }
        _iday = r[6].as<int>();
        _imonth = r[7].as<int>();
        _ihours = r[8].as<int>();
        _imins = r[9].as<int>();
        _isecs = r[10].as<int>();
        _NLayer = r[11].as<int>();
        _CalTm = r[12].as<double>();
        _isInitial = true;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        _isValid = false;
        return false;
    }

    if (!_tlev)
    {
        _tlev = std::make_unique<TLevelObject>(_gid, *_qry, this);
        _isValid = *_tlev;
        if (!_isValid)
        {
            _tlev.reset();
            return false;
        }
    }
    if (!_alev)
    {
        _alev = std::make_unique<ALevelObject>(_gid, *_qry, this);
        _isValid = *_alev;
        if (!_isValid)
        {
            _alev.reset();
            return false;
        }
    }
    if (!_nod)
    {
        _nod = std::make_unique<NodInfoObject>(_gid, *_qry, this);
        _isValid = *_nod;
        if (!_isValid)
        {
            _nod.reset();
            return false;
        }
    }
    if (!_bal)
    {
        _bal = std::make_unique<BalanceObject>(_gid, *_qry, this);
        _isValid = *_bal;
        if (!_isValid)
        {
            _bal.reset();
            return false;
        }
    }

    if (!_obs && _NObs)
    {
        _obs = std::make_unique<ObsNodeObject>(_gid, *_qry, this);
        _isValid = *_obs;
        if (!_isValid)
        {
            _obs.reset();
            return false;
        }
    }
    for (int i = 1; i <= _NS; ++i)
    {
        if (!_sol[i - 1])
        {
            _sol[i - 1] = std::make_unique<SoluteObject>(_gid, *_qry, this, i);
            _isValid = *_sol[i - 1];
            if (!_isValid)
            {
                _sol[i - 1].reset();
                return false;
            }
        }
    }
    if (!_tlev->Save(_path))
    {
        return false;
    }
    if (!_alev->Save(_path))
    {
        return false;
    }
    if (!_nod->Save(_path))
    {
        return false;
    }
    if (!_bal->Save(_path))
    {
        return false;
    }
    if (_NObs && _obs && !_obs->Save(_path))
    {
        return false;
    }
    for (int i = 1; i <= _NS; ++i)
    {
        if (_sol[i - 1] && !_sol[i - 1]->Save(_path))
        {
            return false;
        }
    }
    return true;
}

bool HydrusParameterFilesManager::DropResultFiles()
{
    if (!_isValid)
    {
        return false;
    }
    try
    {
        std::string s("select * from clearhydrusresultbyid($1);");
        pqxx::work w(*_qry);
        w.exec_params1(s, _gid);
        w.commit();
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
    return true;
}

bool HydrusParameterFilesManager::DropCase()
{
    if (!_isValid)
    {
        return false;
    }
    try
    {
        std::string s("select * from removehydruscasebyid($1);");
        pqxx::work w(*_qry);
        w.exec_params1(s, _gid);
        w.commit();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool HydrusParameterFilesManager::DropInputFiles()
{
    if (!_isValid)
    {
        return false;
    }
    try
    {
        std::string s("select * from removehydrusinputparambyid($1);");
        pqxx::work w(*_qry);
        w.exec_params1(s, _gid);
        w.commit();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

std::unique_ptr<std::string> HydrusParameterFilesManager::GetImportResultFilesSQlStatement()
{
    std::unique_ptr<std::string> _usptr;
    if (!_isInitial)
    {
        return _usptr;
    }
    OpenResultFiles();
    if (!_isValid)
    {
        return _usptr;
    }
    std::stringstream strbld;
    if (_err)
    {
        strbld << _errMessage;
    }
    else
    {
        //strbld<<"set constraint_exclusion = on;";
        strbld << "BEGIN TRANSACTION;";
        if (_status == "done")
        {
            strbld << "select * from clearhydrusresultbyid(" << _gid << ");";
        }
        strbld << _tlev->ToSqlStatement(_gid);
        strbld << _alev->ToSqlStatement(_gid);
        strbld << _nod->ToSqlStatement(_gid);
        strbld << _bal->ToSqlStatement(_gid);
        if (_NObs)
        {
            strbld << _obs->ToSqlStatement(_gid);
        }
        for (int i = 0; i < _NS; ++i)
        {
            strbld << _sol[i]->ToSqlStatement(_gid);
        }
        strbld << "select * from updateselectorstatus(" << _gid << ",true);";
        strbld << "COMMIT;";
    }
    _usptr = std::make_unique<std::string>(strbld.str());
    return _usptr;
}

HydrusParameterFilesManager::HydrusParameterFilesManager(unsigned int nlayer, unsigned int ns, unsigned int nobs, unsigned int *iobs, const std::string &status)
{
    _err = false;
    _errMessage = "";
    _Hed = "Welcome to HYDRUS-1D (HydrusExecuter)";
    _NLayer = nlayer;
    _NS = ns;
    _HeadLine = 5;
    _iday = 0;
    _imonth = 0;
    _ihours = 0;
    _imins = 0;
    _isecs = 0;
    _NObs = nobs;
    if (_NObs)
    {
        _iobs = std::make_unique<unsigned int[]>(_NObs);
        if (iobs)
        {
            for (int i = 0; i < _NObs; ++i)
            {
                _iobs[i] = iobs[i];
            }
        }
    }
    _LUnit = "cm";
    _TUnit = "days";
    _MUnit = "mmol";
    _CalTm = std::numeric_limits<double>::max();
    _qry = nullptr;
    _status = status;
    _isValid = true;
    _isInitial = false;
}

bool HydrusParameterFilesManager::ParseSqlARRAY(const std::string &value, unsigned int *p, unsigned int nsize)
{
    std::regex pattern("\\{(.+)\\}");
    std::smatch mat;
    std::regex_search(value, mat, pattern);
    std::stringstream strbld(mat.str(1));
    unsigned int i = 0;
    std::string singlevalue;
    while (i < nsize && getline(strbld, singlevalue, ','))
    {
        p[i++] = stoi(singlevalue);
    }
    return true;
}

void HydrusParameterFilesManager::GetErrMessage(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in)
    {
        _isValid = false;
        return;
    }
    in.seekg(0, std::ios::end);
    int pos = static_cast<int>(in.tellg());
    if (pos == 0) //Empty Error.msg File
    {
        _errMessage = "Unknown Error, the Error.msg has no content";
        in.close();
        return;
    }
    std::unique_ptr<char[]> pbuf(new char[pos + 1]);
    in.seekg(0, std::ios::beg);
    in.read(pbuf.get(), pos);
    in.close();
    pbuf[pos] = '\0';
    Stringhelper s(pbuf.get());
    s.replace('\'', '\"');
    _errMessage = s.simplified().str();
}

void HydrusParameterFilesManager::OpenInputFiles()
{
    std::map<std::string, std::string> dic;
    std::filesystem::path p = _path;
    std::string filename;
    for (const auto &entry : std::filesystem::directory_iterator(p))
    {
        if (entry.is_regular_file())
        {
            filename = entry.path().filename().string();
        }
        else
        {
            continue;
        }
        Stringhelper sh(filename);
        if (sh.compare("selector.in", false) == 0)
        {
            std::string f = std::filesystem::absolute(entry.path()).string();
            if (!_sel)
            {
                _sel = std::make_unique<SelectorObject>(f, this);
                _isValid = *_sel;
                if (!_isValid)
                {
                    _sel.reset();
                    return;
                }
            }
        }
        else if (sh.compare("atmosph.in", false) == 0)
        {
            dic["atmosph"] = std::filesystem::absolute(entry.path()).string();
        }
        else if (sh.compare("profile.dat", false) == 0)
        {
            dic["profile"] = std::filesystem::absolute(entry.path()).string();
        }
    }

    if (!_atm && _sel && dic.find("atmosph") != dic.end())
    {
        _atm = std::make_unique<AtmosphObject>(dic["atmosph"], _sel.get());
        _isValid = *_atm;
        if (!_isValid)
        {
            _atm.reset();
            return;
        }
    }
    if (!_pro && _sel && dic.find("profile") != dic.end())
    {
        _pro = std::make_unique<ProfileObject>(dic["profile"], _sel.get());
        _isValid = *_pro;
        if (!_isValid)
        {
            _pro.reset();
            return;
        }
    }

    if (!_sel || !_atm || !_pro)
    {
        _isValid = false;
    }
    _isInitial = true;
}

void HydrusParameterFilesManager::OpenResultFiles()
{
    std::map<std::string, std::string> dic;
    std::filesystem::path p = _path;
    std::string filename;
    for (const auto &entry : std::filesystem::directory_iterator(p))
    {
        if (entry.is_regular_file())
        {
            filename = entry.path().filename().string();
        }
        else
        {
            continue;
        }
        Stringhelper sh(filename);
        if (sh.compare("error.msg", false) == 0)
        {
            _err = true;
            GetErrMessage(std::filesystem::absolute(entry.path().string()).string());
            return;
        }
        if (sh.compare("t_level.out", false) == 0)
        {
            std::string val = std::filesystem::absolute(entry.path()).string();

            if (!_tlev)
            {
                _tlev = std::make_unique<TLevelObject>(val, this);
                _isValid = *_tlev;
                if (!_isValid)
                {
                    _tlev.reset();
                    return;
                }
            }
        }
        else if (sh.compare("a_level.out", false) == 0)
        {
            dic["alevel"] = std::filesystem::absolute(entry.path()).string();
        }
        else if (sh.compare("nod_inf.out", false) == 0)
        {
            dic["nod_inf"] = std::filesystem::absolute(entry.path()).string();
        }
        else if (_NObs && sh.compare("obs_node.out", false) == 0)
        {
            dic["obs_node"] = std::filesystem::absolute(entry.path()).string();
        }
        else if (sh.compare("balance.out", false) == 0)
        {
            dic["balance"] = std::filesystem::absolute(entry.path()).string();
        }
        else
        {
            for (int i = 1; i <= 10; ++i)
            {
                std::string s = Stringhelper("solute%1.out").arg(i).str();
                if (sh.compare(s, false) == 0)
                {
                    dic[Stringhelper("solute%1").arg(i).str()] = std::filesystem::absolute(entry.path()).string();
                    break;
                }
            }
        }
    }
    if (!_alev && dic.find("alevel") != dic.end())
    {
        _alev = std::make_unique<ALevelObject>(dic["alevel"], this);
        _isValid = *_alev;
        if (!_isValid)
        {
            _alev.reset();
            return;
        }
    }
    if (!_nod && dic.find("nod_inf") != dic.end())
    {
        _nod = std::make_unique<NodInfoObject>(dic["nod_inf"], this);
        _isValid = *_nod;
        if (!_isValid)
        {
            _nod.reset();
            return;
        }
    }
    if (!_bal && dic.find("balance") != dic.end())
    {
        _bal = std::make_unique<BalanceObject>(dic["balance"], this);
        _isValid = *_bal;
        if (!_isValid)
        {
            _bal.reset();
            return;
        }
    }
    if (!_obs && dic.find("obs_node") != dic.end())
    {
        _obs = std::make_unique<ObsNodeObject>(dic["obs_node"], this);
        _isValid = *_obs;
        if (!_isValid)
        {
            _obs.reset();
            return;
        }
    }
    for (int i = 1; i <= 10; ++i)
    {
        std::string key = Stringhelper("solute%1").arg(i).str();
        if (!_sol[i - 1] && dic.find(key) != dic.end())
        {
            _sol[i - 1] = std::make_unique<SoluteObject>(dic[key], this);
            _isValid = *_sol[i - 1];
            if (!_isValid)
            {
                _sol[i - 1].reset();
                return;
            }
        }
    }
    if (!_tlev || !_alev || !_nod || !_bal)
    {
        _isValid = false;
    }
}
