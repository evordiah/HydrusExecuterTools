
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

#include <memory>
#include <pqxx/pqxx>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include "HydrusParameterFilesManager.h"
#include "TLevelObject.h"
#include "FFmt.h"
#include "Stringhelper.h"

TLevelObject::TLevelObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _isValid = open(filename);
    if (_isValid)
    {
        _parent->_HeadLine = _HeadLine;
        _parent->_Hed = _Hed;
        _parent->_iday = _iday;
        _parent->_imonth = _imonth;
        _parent->_ihours = _ihours;
        _parent->_imins = _imins;
        _parent->_isecs = _isecs;
    }
}

TLevelObject::TLevelObject(int gid, pqxx::connection &qry, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _isValid = open(gid, qry);
    if (_isValid)
    {
        _HeadLine = _parent->_HeadLine;
        _Hed = _parent->_Hed;
        _iday = _parent->_iday;
        _imonth = _parent->_imonth;
        _ihours = _parent->_ihours;
        _imins = _parent->_imins;
        _isecs = _parent->_isecs;
    }
}

TLevelObject::~TLevelObject()
{
    //dtor
}

/*-------------Hydrus OUTPUT.FOR output format for Description------------
//     format(/
//     !'       Time          rTop        rRoot        vTop         vRoot
//     !       vBot       sum(rTop)   sum(rRoot)    sum(vTop)   sum(vRoot)
//     !    sum(vBot)      hTop         hRoot        hBot        RunOff
//     ! sum(RunOff)     Volume     sum(Infil)    sum(Evap) TLevel Cum(WTr
//     !ans)  SnowLayer'/
//     !'        [T]         [L/T]        [L/T]        [L/T]        [L/T]
//     !       [L/T]         [L]          [L]          [L]         [L]
//     !       [L]         [L]           [L]         [L]          [L/T]
//     !      [L]          [L]          [L]          [L]'/)
-----------------------------------------------------------------------*/
bool TLevelObject::Save(const std::string &path)
{
    if (!_isValid)
    {
        return false;
    }
    std::filesystem::path p = path;
    if (!std::filesystem::exists(p))
    {
        if (!std::filesystem::create_directories(p))
        {
            return false;
        }
    }
    std::ofstream out((std::filesystem::absolute(p) / "T_Level.out").string());
    if (!out)
    {
        return false;
    }
    out << " ******* Program HYDRUS" << std::endl;
    out << std::left;
    if (_HeadLine == 4)
    {
        out << " ******* " << std::setw(72) << _Hed << std::endl;
    }
    else
    {
        out << " ******* " << std::endl
            << ' ' << std::setw(72) << _Hed << std::endl;
    }
    out << std::right;
    //Hydrus output head format
    //format(' Date: ',i3,'.',i2,'.','    Time: ',i3,':',i2,':',i2)
    out << " Date: " << std::setw(3) << _iday << '.'
        << std::setw(2) << _imonth << '.'
        << "    Time: " << std::setw(3) << _ihours << ':'
        << std::setw(2) << _imins << ':'
        << std::setw(2) << _isecs << std::endl;
    out << std::left;
    out << " Units: L = " << std::setw(5) << _parent->LUnit()
        << ", T = " << std::setw(5) << _parent->TUnit()
        << ", M = " << std::setw(5) << _parent->MUnit() << std::endl;
    out << std::right;
    out << std::endl;
    out << "       Time          rTop        rRoot        vTop         vRoot "
           "       vBot       sum(rTop)   sum(rRoot)    sum(vTop)   sum(vRoot)"
           "    sum(vBot)      hTop         hRoot        hBot        RunOff   "
           " sum(RunOff)     Volume     sum(Infil)    sum(Evap) TLevel Cum(WTr"
           "ans)  SnowLayer"
        << std::endl;
    out << "        [T]         [L/T]        [L/T]        [L/T]        [L/T] "
           "       [L/T]         [L]          [L]          [L]         [L]    "
           "       [L]         [L]           [L]         [L]          [L/T]   "
           "      [L]          [L]          [L]          [L]"
        << std::endl;
    out << std::endl;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        out << **it << std::endl;
    }
    out << "end" << std::endl;
    out.close();
    return true;
}

bool TLevelObject::Save(std::ostream &out)
{
    if (!_isValid || !out)
    {
        return false;
    }
    out << " ******* Program HYDRUS" << std::endl;
    out << std::left;
    if (_HeadLine == 4)
    {
        out << " ******* " << std::setw(72) << _Hed << std::endl;
    }
    else
    {
        out << " ******* " << std::endl
            << ' ' << std::setw(72) << _Hed << std::endl;
    }
    out << std::right;
    //Hydrus output head format
    //format(' Date: ',i3,'.',i2,'.','    Time: ',i3,':',i2,':',i2)
    out << " Date: " << std::setw(3) << _iday << '.'
        << std::setw(2) << _imonth << '.'
        << "    Time: " << std::setw(3) << _ihours << ':'
        << std::setw(2) << _imins << ':'
        << std::setw(2) << _isecs << std::endl;
    out << std::left;
    out << " Units: L = " << std::setw(5) << _parent->LUnit()
        << ", T = " << std::setw(5) << _parent->TUnit()
        << ", M = " << std::setw(5) << _parent->MUnit() << std::endl;
    out << std::right;
    out << std::endl;
    out << "       Time          rTop        rRoot        vTop         vRoot "
           "       vBot       sum(rTop)   sum(rRoot)    sum(vTop)   sum(vRoot)"
           "    sum(vBot)      hTop         hRoot        hBot        RunOff   "
           " sum(RunOff)     Volume     sum(Infil)    sum(Evap) TLevel Cum(WTr"
           "ans)  SnowLayer"
        << std::endl;
    out << "        [T]         [L/T]        [L/T]        [L/T]        [L/T] "
           "       [L/T]         [L]          [L]          [L]         [L]    "
           "       [L]         [L]           [L]         [L]          [L/T]   "
           "      [L]          [L]          [L]          [L]"
        << std::endl;
    out << std::endl;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        out << **it << std::endl;
    }
    out << "end" << std::endl;
    return true;
}

std::string TLevelObject::ToSqlStatement(const int gid)
{
    std::stringstream out;
    Stringhelper s("update selector set iday=%1,imonth=%2,ihours=%3,imins=%4,isecs=%5"
                   " where gid=%6 ;");
    s.arg(_iday).arg(_imonth).arg(_ihours).arg(_imins).arg(_isecs).arg(gid);
    out << s.str();
    out << "INSERT INTO t_level("
           "gid, tm, rtop, rroot, vtop, vroot, vbot, sr_top, sr_root, "
           "sv_top, sv_root, sv_bot, htop, hroot, hbot, runoff, s_runoff, "
           "volume, s_infil, s_evap, tlevel, s_wtrans, snowlayer) VALUES";
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        out << "(" << gid << ","
            << fwzformat::SqlValueExpression << **it << "),";
    }
    std::string sql = out.str();
    sql.back() = ';';
    return sql;
}

bool TLevelObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in)
    {
        return false;
    }
    //parse the head lines
    if (!ParseHead(in))
    {
        return false;
    }
    int i = 0;
    std::string line;
    while (i++ < 4)
    {
        std::getline(in, line);
    }

    std::unique_ptr<TLevelRecord> pRec;
    std::getline(in, line);
    if (line.substr(0, 3) == "end")
    {
        return false;
    }
    pRec = std::make_unique<TLevelRecord>(line.c_str());
    _Recs.push_back(std::move(pRec));
    while (true)
    {
        std::getline(in, line);
        if (line.substr(0, 3) == "end")
        {
            break;
        }
        pRec = std::make_unique<TLevelRecord>(line.c_str());
        if (pRec->Time == _Recs.back()->Time)
        {
            pRec->Time += 1e-5;
        }
        _Recs.push_back(std::move(pRec));
    }
    return true;
}

bool TLevelObject::open(int gid, pqxx::connection &qry)
{
    std::string sql("select tm, rtop, rroot, vtop, vroot, vbot, sr_top, sr_root, "
                    "sv_top, sv_root, sv_bot, htop, hroot, hbot, runoff, s_runoff, "
                    "volume, s_infil, s_evap, tlevel, s_wtrans, snowlayer "
                    "from t_level where gid=$1 order by tm;");
    try
    {
        pqxx::work w(qry);
        pqxx::result r = w.exec_params(sql, gid);
        w.commit();
        if (r.empty())
        {
            return false;
        }
        std::unique_ptr<TLevelRecord> pRec;
        for (auto it = r.begin(); it != r.end(); ++it)
        {
            pRec = std::make_unique<TLevelRecord>(it);
            _Recs.push_back(std::move(pRec));
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool TLevelObject::ParseHead(std::istream &in)
{
    std::string line;
    //ignore the first line
    getline(in, line);
    //parse the second line
    getline(in, line);
    if (line.back() == '\r')
    {
        line.pop_back();
    }
    if (line.length() > 72)
    {
        _HeadLine = 4;
        _Hed = line.substr(line.length() - 72, 72);
    }
    else
    {
        _HeadLine = 5;
        getline(in, line);
        if (line.back() == '\r')
        {
            line.pop_back();
        }
        _Hed = line.substr(line.length() - 72, 72);
    }
    getline(in, line);
    //format(' Date: ',i3,'.',i2,'.','    Time: ',i3,':',i2,':',i2)
    char *pbuf = const_cast<char *>(line.c_str() + 7);
    std::string tmp = std::string(pbuf, 3);
    _iday = std::stoi(std::string(pbuf, 3));
    pbuf += 4;
    _imonth = std::stoi(std::string(pbuf, 2));
    pbuf += 13;
    _ihours = std::stoi(std::string(pbuf, 3));
    pbuf += 4;
    _imins = std::stoi(std::string(pbuf, 2));
    pbuf += 3;
    _isecs = std::stoi(std::string(pbuf, 2));
    getline(in, line);
    //    auto pos=line.find("Units");
    //    pbuf=const_cast<char*>(line.c_str()+pos+11);
    //    _LUnit=std::string(pbuf,5);
    //    pbuf+=11;
    //    _TUnit=std::string(pbuf,5);
    //    pbuf+=11;
    //    _MUnit=std::string(pbuf,5);
    return true;
}

/*-------------Hydrus OUTPUT.FOR output format for each line in T_Level.out----------
//format(f13.4,11e13.5,2e13.5,5e13.5,i7,e13.5,f11.3)
-----------------------------------------------------------------------------------*/

TLevelObject::TLevelRecord::TLevelRecord(const char *pline)
{
    int index[22] =
        {
            13, 13, 13, 13, 13,
            13, 13, 13, 13, 13,
            13, 13, 13, 13, 13,
            13, 13, 13, 13, 7,
            13, 11};
    char split[22][14] = {0};
    char *psrc = const_cast<char *>(pline);
    for (int i = 0; i < 22; i++)
    {
        std::memcpy(&split[i][0], psrc, index[i]);
        psrc += index[i];
    }
    Time = atof(split[0]);
    rTop = atof(split[1]);
    rRoot = atof(split[2]);
    vTop = atof(split[3]);
    vRoot = atof(split[4]);
    vBot = atof(split[5]);
    sum_rTop = atof(split[6]);
    sum_rRoot = atof(split[7]);
    sum_vTop = atof(split[8]);
    sum_vRoot = atof(split[9]);
    sum_vBot = atof(split[10]);
    hTop = atof(split[11]);
    hRoot = atof(split[12]);
    hBot = atof(split[13]);
    RunOff = atof(split[14]);
    sum_RunOff = atof(split[15]);
    Volume = atof(split[16]);
    sum_Infil = atof(split[17]);
    sum_Evap = atof(split[18]);
    TLevel = atoi(split[19]);
    Sum_WTrans = atof(split[20]);
    SnowLayer = atof(split[21]);
}

TLevelObject::TLevelRecord::TLevelRecord(pqxx::row &row)
{
    Time = row[0].as<double>();
    rTop = row[1].as<double>();
    rRoot = row[2].as<double>();
    vTop = row[3].as<double>();
    vRoot = row[4].as<double>();
    vBot = row[5].as<double>();
    sum_rTop = row[6].as<double>();
    sum_rRoot = row[7].as<double>();
    sum_vTop = row[8].as<double>();
    sum_vRoot = row[9].as<double>();
    sum_vBot = row[10].as<double>();
    hTop = row[11].as<double>();
    hRoot = row[12].as<double>();
    hBot = row[13].as<double>();
    RunOff = row[14].as<double>();
    sum_RunOff = row[15].as<double>();
    Volume = row[16].as<double>();
    sum_Infil = row[17].as<double>();
    sum_Evap = row[18].as<double>();
    TLevel = row[19].as<int>();
    Sum_WTrans = row[20].as<double>();
    SnowLayer = row[21].as<double>();
}

std::ostream &operator<<(std::ostream &os, const TLevelObject::TLevelRecord &trec)
{
    os << std::fixed << std::setprecision(4);
    os << std::setw(13) << trec.Time;
    os << std::setprecision(5);
    os << std::setw(13) << fwzformat::fortranE2 << trec.rTop;
    os << std::setw(13) << fwzformat::fortranE2 << trec.rRoot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.vTop;
    os << std::setw(13) << fwzformat::fortranE2 << trec.vRoot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.vBot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_rTop;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_rRoot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_vTop;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_vRoot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_vBot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.hTop;
    os << std::setw(13) << fwzformat::fortranE2 << trec.hRoot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.hBot;
    os << std::setw(13) << fwzformat::fortranE2 << trec.RunOff;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_RunOff;
    os << std::setw(13) << fwzformat::fortranE2 << trec.Volume;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_Infil;
    os << std::setw(13) << fwzformat::fortranE2 << trec.sum_Evap;
    os << std::setw(7) << trec.TLevel;
    os << std::setw(13) << fwzformat::fortranE2 << trec.Sum_WTrans;
    os << std::setw(11) << std::setprecision(3) << trec.SnowLayer;
    return os;
}

template <>
std::ostream &fwzformat::operator<<(const fwzformat::ffmt_proxy &q, const TLevelObject::TLevelRecord &rhs)
{
    return q.os << std::fixed << std::setprecision(5)
                << rhs.Time << ","
                << fwzformat::fortranE2 << rhs.rTop << ","
                << fwzformat::fortranE2 << rhs.rRoot << ","
                << fwzformat::fortranE2 << rhs.vTop << ","
                << fwzformat::fortranE2 << rhs.vRoot << ","
                << fwzformat::fortranE2 << rhs.vBot << ","
                << fwzformat::fortranE2 << rhs.sum_rTop << ","
                << fwzformat::fortranE2 << rhs.sum_rRoot << ","
                << fwzformat::fortranE2 << rhs.sum_vTop << ","
                << fwzformat::fortranE2 << rhs.sum_vRoot << ","
                << fwzformat::fortranE2 << rhs.sum_vBot << ","
                << fwzformat::fortranE2 << rhs.hTop << ","
                << fwzformat::fortranE2 << rhs.hRoot << ","
                << fwzformat::fortranE2 << rhs.hBot << ","
                << fwzformat::fortranE2 << rhs.RunOff << ","
                << fwzformat::fortranE2 << rhs.sum_RunOff << ","
                << fwzformat::fortranE2 << rhs.Volume << ","
                << fwzformat::fortranE2 << rhs.sum_Infil << ","
                << fwzformat::fortranE2 << rhs.sum_Evap << ","
                << rhs.TLevel << ","
                << fwzformat::fortranE2 << rhs.Sum_WTrans << ","
                << std::setprecision(3) << rhs.SnowLayer;
}
