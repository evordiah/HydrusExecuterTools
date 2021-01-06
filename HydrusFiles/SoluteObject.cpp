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
#include "FFmt.h"
#include "SoluteObject.h"
#include "Stringhelper.h"

SoluteObject::SoluteObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _NObs = _parent->NumofObsNodes();
    _NObs = _NObs > 3 ? 3 : _NObs;
    _isValid = open(filename);
}

SoluteObject::SoluteObject(int gid, pqxx::connection &qry, HydrusParameterFilesManager *parent, const int index)
{
    _parent = parent;
    _FileIndex = index;
    _NObs = _parent->NumofObsNodes();
    _NObs = _NObs > 3 ? 3 : _NObs;
    _isValid = open(gid, qry);
}

SoluteObject::~SoluteObject()
{
    //dtor
}

bool SoluteObject::Save(const std::string &path)
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
    std::string Filename = Stringhelper("solute%1.out").arg(_FileIndex).str();
    std::ofstream out((std::filesystem::absolute(p) / Filename).string());
    if (!out)
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
    out << " All solute fluxes and cumulative solute fluxes are positi"
           "ve into the region"
        << std::endl
        << std::endl;
    out << "       Time         cvTop        cvBot      Sum(cvTop)   Sum(cvBo"
           "t)     cvCh0        cvCh1         cTop        cRoot         cBot  "
           "      cvRoot    Sum(cvRoot)  Sum(cvNEql) TLevel      cGWL        c"
           "RunOff   Sum(cRunOff)    (cv(i),    Sum(cv(i)), i=1,NObs)"
        << std::endl;
    out << "        [T]        [M/L2/T]     [M/L2/T]      [M/L2]       [M/L2]"
           "       [M/L2]      [M/L2]        [M/L3]      [M/L3]        [M/L3] "
           "     [M/L2/T]      [M/L2]       [M/L2]              [M/L3]        "
           "[M/L2]      [M/L3]      [M/L2/T]      [M/L2]"
        << std::endl;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        SaveLine(out, **it);
    }
    out << "end" << std::endl;
    out.close();
    return true;
}

bool SoluteObject::Save(std::ostream &out)
{
    if (!_isValid || !out)
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
    out << " All solute fluxes and cumulative solute fluxes are positi"
           "ve into the region"
        << std::endl
        << std::endl;
    out << "       Time         cvTop        cvBot      Sum(cvTop)   Sum(cvBo"
           "t)     cvCh0        cvCh1         cTop        cRoot         cBot  "
           "      cvRoot    Sum(cvRoot)  Sum(cvNEql) TLevel      cGWL        c"
           "RunOff   Sum(cRunOff)    (cv(i),    Sum(cv(i)), i=1,NObs)"
        << std::endl;
    out << "        [T]        [M/L2/T]     [M/L2/T]      [M/L2]       [M/L2]"
           "       [M/L2]      [M/L2]        [M/L3]      [M/L3]        [M/L3] "
           "     [M/L2/T]      [M/L2]       [M/L2]              [M/L3]        "
           "[M/L2]      [M/L3]      [M/L2/T]      [M/L2]"
        << std::endl;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        SaveLine(out, **it);
    }
    out << "end" << std::endl;
    return true;
}

std::string SoluteObject::ToSqlStatement(const int gid)
{
    std::stringstream out;
    out << "INSERT INTO solute" << _FileIndex << "(gid, "
                                                 "tm, cvtop, cvbot, s_cvtop, s_cvbot, s_cvch0, s_cvch1, ctop, "
                                                 "croot, cbot, cvroot, s_cvroot, s_cvneql, t_level, cgwl, crunoff,s_crunoff";
    for (int i = 1; i <= _NObs; ++i)
    {
        out << ",cv" << i << ",s_cv" << i;
    }
    out << ") values";
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        out << "(" << gid << ","
            << ToSqlStatement(**it) << "),";
    }
    std::string sql = out.str();
    sql.back() = ';';
    return sql;
}

bool SoluteObject::open(const std::string &filename)
{
    //QFileInfo fi(filename.c_str());
    //QString baseName=fi.baseName();
    std::filesystem::path fi = filename;
    std::string baseName = fi.stem().string();
    _FileIndex = std::stoi(baseName.substr(6));
    std::ifstream in(filename);
    if (!in)
    {
        return false;
    }
    //ignore the head lines
    int i = 0;
    std::string line;
    while (i++ < 4)
    {
        std::getline(in, line);
    }
    std::unique_ptr<SoluteRecord> pRec;
    while (true)
    {
        std::getline(in, line);
        if (line.substr(0, 3) == "end")
        {
            break;
        }
        pRec = std::make_unique<SoluteRecord>(line.c_str(), _NObs);
        _Recs.push_back(std::move(pRec));
    }
    return true;
}

bool SoluteObject::open(int gid, pqxx::connection &qry)
{
    std::stringstream strbld;
    strbld << "select tm, cvtop, cvbot, s_cvtop, s_cvbot, s_cvch0, s_cvch1, ctop, "
              "croot, cbot, cvroot, s_cvroot, s_cvneql, t_level, cgwl, crunoff,s_crunoff";
    for (int i = 1; i <= _NObs; ++i)
    {
        strbld << ",cv" << i << ",s_cv" << i;
    }
    strbld << " from solute" << _FileIndex << " where gid=" << gid << " order by tm;";
    try
    {
        pqxx::work w(qry);
        pqxx::result r = w.exec(strbld.str());
        w.commit();
        if (r.empty())
        {
            return false;
        }
        std::unique_ptr<SoluteRecord> pRec;
        for (auto it = r.begin(); it != r.end(); ++it)
        {
            pRec = std::make_unique<SoluteRecord>(it, _NObs);
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

//format(f14.4,12e13.5,i8,e13.5,8e13.5)
void SoluteObject::SaveLine(std::ostream &os, const SoluteObject::SoluteRecord &srec)
{
    os << std::fixed << std::setprecision(4);
    os << std::setw(14) << srec.Time;
    os << std::setprecision(5);
    os << std::setw(13) << fwzformat::fortranE2 << srec.cvTop;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cvBot;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cvTop;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cvBot;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cvCh0;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cvCh1;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cTop;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cRoot;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cBot;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cvRoot;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cvRoot;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cvNEql;
    os << std::setw(8) << srec.TLevel;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cGWL;
    os << std::setw(13) << fwzformat::fortranE2 << srec.cRunOff;
    os << std::setw(13) << fwzformat::fortranE2 << srec.sum_cRunOff;
    for (int i = 0; i < _NObs; ++i)
    {
        os << std::setw(13) << fwzformat::fortranE2 << srec.cv[i];
        os << std::setw(13) << fwzformat::fortranE2 << srec.sumcv[i];
    }
    os << std::endl;
}

std::string SoluteObject::ToSqlStatement(const SoluteObject::SoluteRecord &srec)
{
    std::stringstream strbld;
    strbld << std::fixed << std::setprecision(4);
    strbld << srec.Time << ",";
    strbld << std::setprecision(5);
    strbld << fwzformat::fortranE2 << srec.cvTop << ",";
    strbld << fwzformat::fortranE2 << srec.cvBot << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cvTop << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cvBot << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cvCh0 << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cvCh1 << ",";
    strbld << fwzformat::fortranE2 << srec.cTop << ",";
    strbld << fwzformat::fortranE2 << srec.cRoot << ",";
    strbld << fwzformat::fortranE2 << srec.cBot << ",";
    strbld << fwzformat::fortranE2 << srec.cvRoot << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cvRoot << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cvNEql << ",";
    strbld << srec.TLevel << ",";
    strbld << fwzformat::fortranE2 << srec.cGWL << ",";
    strbld << fwzformat::fortranE2 << srec.cRunOff << ",";
    strbld << fwzformat::fortranE2 << srec.sum_cRunOff;
    for (int i = 0; i < _NObs; ++i)
    {
        strbld << "," << fwzformat::fortranE2 << srec.cv[i];
        strbld << "," << fwzformat::fortranE2 << srec.sumcv[i];
    }
    return strbld.str();
}

/*-------------Hydrus OUTPUT.FOR output format for each line in solute.out----------
format(f14.4,12e13.5,i8,e13.5,8e13.5)
-----------------------------------------------------------------------------------*/

SoluteObject::SoluteRecord::SoluteRecord(const char *pline, const int NOBS)
{
    int index[23] =
        {
            14, 13, 13, 13, 13,
            13, 13, 13, 13, 13,
            13, 13, 13, 8, 13,
            13, 13, 13, 13, 13,
            13, 13, 13};
    char split[23][15] = {{0}};
    char *psrc = const_cast<char *>(pline);
    for (int i = 0; i < 17; i++)
    {
        std::memcpy(&split[i][0], psrc, index[i]);
        psrc += index[i];
    }
    for (int i = 0; i < NOBS * 2; ++i)
    {
        std::memcpy(&split[17 + i][0], psrc, index[17 + i]);
        psrc += index[17 + i];
    }
    Time = atof(split[0]);
    cvTop = atof(split[1]);
    cvBot = atof(split[2]);
    sum_cvTop = atof(split[3]);
    sum_cvBot = atof(split[4]);
    sum_cvCh0 = atof(split[5]);
    sum_cvCh1 = atof(split[6]);
    cTop = atof(split[7]);
    cRoot = atof(split[8]);
    cBot = atof(split[9]);
    cvRoot = atof(split[10]);
    sum_cvRoot = atof(split[11]);
    sum_cvNEql = atof(split[12]);
    TLevel = atoi(split[13]);
    cGWL = atof(split[14]);
    cRunOff = atof(split[15]);
    sum_cRunOff = atof(split[16]);
    for (int i = 0; i < NOBS; ++i)
    {
        cv[i] = atof(split[17 + i * 2]);
        sumcv[i] = atof(split[18 + i * 2]);
    }
}

SoluteObject::SoluteRecord::SoluteRecord(pqxx::row &row, const int NOBS)
{
    Time = row[0].as<double>();
    cvTop = row[1].as<double>();
    cvBot = row[2].as<double>();
    sum_cvTop = row[3].as<double>();
    sum_cvBot = row[4].as<double>();
    sum_cvCh0 = row[5].as<double>();
    sum_cvCh1 = row[6].as<double>();
    cTop = row[7].as<double>();
    cRoot = row[8].as<double>();
    cBot = row[9].as<double>();
    cvRoot = row[10].as<double>();
    sum_cvRoot = row[11].as<double>();
    sum_cvNEql = row[12].as<double>();
    TLevel = row[13].as<int>();
    cGWL = row[14].as<double>();
    cRunOff = row[15].as<double>();
    sum_cRunOff = row[16].as<double>();
    for (int i = 0; i < NOBS; ++i)
    {
        cv[i] = row[17 + i * 2].as<double>();
        sumcv[i] = row[18 + i * 2].as<double>();
    }
}
