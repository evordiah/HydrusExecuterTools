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
#include <sstream>
#include <string>
#include "HydrusParameterFilesManager.h"
#include "ALevelObject.h"
#include "FFmt.h"

ALevelObject::ALevelObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _isValid = open(filename);
}

ALevelObject::ALevelObject(int gid, pqxx::connection &qry, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _isValid = open(gid, qry);
}

ALevelObject::~ALevelObject()
{
    //dtor
}

/*-------------Hydrus OUTPUT.FOR output format for Description------------
  format(//
     !'   Time         sum(rTop)     sum(rRoot)    sum(vTop)     sum(vRo
     !ot)     sum(vBot)    hTop       hRoot      hBot      A-level'/
     !'    [T]           [L]           [L]           [L]           [L]
     !          [L]        [L]         [L]       [L] '/)
-----------------------------------------------------------------------*/
bool ALevelObject::Save(const std::string &path)
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
    std::ofstream out((std::filesystem::absolute(p) / "A_Level.out").string());
    if (!out)
    {
        return false;
    }
    out << std::endl
        << std::endl;
    out << "   Time         sum(rTop)     sum(rRoot)    sum(vTop)     sum(vRo"
           "ot)     sum(vBot)    hTop       hRoot      hBot      A-level"
        << std::endl;
    out << "    [T]           [L]           [L]           [L]           [L]  "
           "          [L]        [L]         [L]       [L] "
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

bool ALevelObject::Save(std::ostream &out)
{
    if (!_isValid || !out)
    {
        return false;
    }
    out << std::endl
        << std::endl;
    out << "   Time         sum(rTop)     sum(rRoot)    sum(vTop)     sum(vRo"
           "ot)     sum(vBot)    hTop       hRoot      hBot      A-level"
        << std::endl;
    out << "    [T]           [L]           [L]           [L]           [L]  "
           "          [L]        [L]         [L]       [L] "
        << std::endl;
    out << std::endl;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        out << **it << std::endl;
    }
    out << "end" << std::endl;
    return true;
}

std::string ALevelObject::ToSqlStatement(const int gid)
{
    std::stringstream out;
    out << "INSERT INTO a_level("
           "gid, tm, sr_top, sr_root, sv_top, "
           "sv_root, sv_bot, htop, hroot, hbot, alevel) VALUES";
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        out << "(" << gid << ","
            << fwzformat::SqlValueExpression << **it << "),";
    }
    std::string sql = out.str();
    sql.back() = ';';
    return sql;
}

bool ALevelObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in)
    {
        return false;
    }
    //ignore the first five lines
    std::string line;
    int i = 0;
    while (i++ < 5)
    {
        std::getline(in, line);
    }
    std::unique_ptr<ALevelRecord> pRec;
    std::getline(in, line);
    if (line.substr(0, 3) == "end")
    {
        return false;
    }
    pRec = std::make_unique<ALevelRecord>(line.c_str());
    _Recs.push_back(std::move(pRec));
    while (true)
    {
        std::getline(in, line);
        if (line.substr(0, 3) == "end")
        {
            break;
        }
        pRec = std::make_unique<ALevelRecord>(line.c_str());
        if (pRec->Time == _Recs.back()->Time)
        {
            pRec->Time += 1e-6;
        }
        _Recs.push_back(std::move(pRec));
    }
    return true;
}

bool ALevelObject::open(int gid, pqxx::connection &qry)
{
    try
    {
        pqxx::work w(qry);
        std::string sql = "select tm, sr_top, sr_root, sv_top, "
                          "sv_root, sv_bot, htop, hroot, hbot, alevel "
                          "from a_level where gid=$1 order by tm;";
        pqxx::result r = w.exec_params(sql, gid);
        w.commit();
        if (r.empty())
        {
            return false;
        }
        std::unique_ptr<ALevelRecord> pRec;
        for (auto it = r.begin(); it != r.end(); ++it)
        {
            pRec = std::make_unique<ALevelRecord>(it);
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

/*-------------Hydrus OUTPUT.FOR output format for each line in A_Level.out----------
//format(f12.5,5e14.6,3f11.3,i8)
-----------------------------------------------------------------------------------*/

ALevelObject::ALevelRecord::ALevelRecord(const char *pline)
{
    int index[10] =
        {
            12, 14, 14, 14, 14,
            14, 11, 11, 11, 8};
    char split[10][15] = {{0}};
    char *psrc = const_cast<char *>(pline);
    for (int i = 0; i < 10; i++)
    {
        std::memcpy(&split[i][0], psrc, index[i]);
        psrc += index[i];
    }
    Time = atof(split[0]);
    sum_rTop = atof(split[1]);
    sum_rRoot = atof(split[2]);
    sum_vTop = atof(split[3]);
    sum_vRoot = atof(split[4]);
    sum_vBot = atof(split[5]);
    hTop = atof(split[6]);
    hRoot = atof(split[7]);
    hBot = atof(split[8]);
    ALevel = atoi(split[9]);
}

ALevelObject::ALevelRecord::ALevelRecord(const pqxx::row &row)
{
    Time = row[0].as<double>();
    sum_rTop = row[1].as<double>();
    sum_rRoot = row[2].as<double>();
    sum_vTop = row[3].as<double>();
    sum_vRoot = row[4].as<double>();
    sum_vBot = row[5].as<double>();
    hTop = row[6].as<double>();
    hRoot = row[7].as<double>();
    hBot = row[8].as<double>();
    ALevel = row[9].as<int>();
}

std::ostream &operator<<(std::ostream &os, const ALevelObject::ALevelRecord &arec)
{
    os << std::fixed << std::setprecision(5);
    os << std::setw(12) << arec.Time;
    os << std::setprecision(6);
    os << std::setw(14) << fwzformat::fortranE2 << arec.sum_rTop;
    os << std::setw(14) << fwzformat::fortranE2 << arec.sum_rRoot;
    os << std::setw(14) << fwzformat::fortranE2 << arec.sum_vTop;
    os << std::setw(14) << fwzformat::fortranE2 << arec.sum_vRoot;
    os << std::setw(14) << fwzformat::fortranE2 << arec.sum_vBot;
    os << std::setprecision(3);
    os << std::setw(11) << arec.hTop;
    os << std::setw(11) << arec.hRoot;
    os << std::setw(11) << arec.hBot;
    os << std::setw(8) << arec.ALevel;
    return os;
}

template <>
std::ostream &fwzformat::operator<<(const fwzformat::ffmt_proxy &q, const ALevelObject::ALevelRecord &rhs)
{
    return q.os << std::fixed << std::setprecision(5)
                << rhs.Time << ","
                << std::setprecision(6)
                << fwzformat::fortranE2 << rhs.sum_rTop << ","
                << fwzformat::fortranE2 << rhs.sum_rRoot << ","
                << fwzformat::fortranE2 << rhs.sum_vTop << ","
                << fwzformat::fortranE2 << rhs.sum_vRoot << ","
                << fwzformat::fortranE2 << rhs.sum_vBot << ","
                << std::setprecision(3)
                << rhs.hTop << ","
                << rhs.hRoot << ","
                << rhs.hBot << ","
                << rhs.ALevel;
}
