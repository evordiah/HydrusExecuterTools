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
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include "SelectorObject.h"
#include "FFmt.h"
#include "HydrusParameterFilesManager.h"
#include "ProfileObject.h"
#include "Stringhelper.h"

ProfileObject::ProfileObject(const std::string &filename, SelectorObject *sel)
{
    _sel = sel;
    Initial();
    _isValid = open(filename);
    if (_isValid)
    {
        _sel->_NumNP = _NumNP;
        _sel->_NObs = _NObs;
        if (_NObs)
        {
            _sel->_iObs = std::make_unique<int[]>(_NObs);
            for (int i = 0; i < _NObs; ++i)
            {
                _sel->_iObs[i] = _iObs[i];
            }
        }
        _sel->UpdateObsInfo();
    }
}

ProfileObject::ProfileObject(int gid, pqxx::connection &qry, SelectorObject *sel)
{
    _sel = sel;
    Initial();
    _isValid = open(gid, qry);
    if (_isValid)
    {
        _iTemp = _sel->lChem ? 1 : 0;
        if (!_sel->lChem || _sel->lEquil)
        {
            _iEquil = 1;
        }
        else
        {
            _iEquil = 0;
        }
    }
}

ProfileObject::~ProfileObject()
{
}

bool ProfileObject::Save(const std::string &path)
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
    std::ofstream out((std::filesystem::absolute(p) / "PROFILE.DAT").string());
    if (!out)
    {
        return false;
    }
    //line 0
    out << "Pcp_File_Version=4" << std::endl;
    //line 1
    out << 2 << std::endl;
    //line 2
    out << "    1  0.000000e+000  1.000000e+000  1.000000e+000" << std::endl;
    auto precision = out.precision();
    out << "    2" << std::setw(15) << std::fixed << std::setprecision(6) << fwzformat::SE3 << _x[_NumNP - 1] << "  1.000000e+000  1.000000e+000" << std::endl;
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    //line 3
    out << ' ' << _NumNP
        << ' ' << _NS
        << ' ' << _iTemp
        << ' ' << _iEquil;
    if (!_iEquil)
    {
        out << " x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc           SConc";
    }
    else
    {
        out << " x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc ";
    }
    out << std::endl;
    //line 4
    if (!_NS)
    {
        for (int i = 0; i < _NumNP; ++i)
        {
            SaveLine(out, i);
        }
    }
    else
    {
        for (int i = 0; i < _NumNP; ++i)
        {
            SaveLine(out, i, _NS);
        }
    }
    //line 5
    out << std::setw(5) << _NObs << std::endl;
    //line 6
    for (int i = 0; i < _NObs; ++i)
    {
        out << std::setw(4) << _iObs[i] << ' ';
    }
    out << std::endl;
    out.close();
    return true;
}

bool ProfileObject::Save(std::ostream &out)
{
    if (!_isValid || !out)
    {
        return false;
    }
    //line 0
    out << "Pcp_File_Version=4" << std::endl;
    //line 1
    out << 2 << std::endl;
    //line 2
    out << "    1  0.000000e+000  1.000000e+000  1.000000e+000" << std::endl;
    auto precision = out.precision();
    out << "    2" << std::setw(15) << std::fixed << std::setprecision(6) << fwzformat::SE3 << _x[_NumNP - 1] << "  1.000000e+000  1.000000e+000" << std::endl;
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    //line 3
    out << ' ' << _NumNP
        << ' ' << _NS
        << ' ' << _iTemp
        << ' ' << _iEquil;
    if (!_iEquil)
    {
        out << " x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc           SConc";
    }
    else
    {
        out << " x         h      Mat  Lay      Beta           Axz            Bxz            Dxz          Temp          Conc ";
    }
    out << std::endl;
    //line 4
    if (!_NS)
    {
        for (int i = 0; i < _NumNP; ++i)
        {
            SaveLine(out, i);
        }
    }
    else
    {
        for (int i = 0; i < _NumNP; ++i)
        {
            SaveLine(out, i, _NS);
        }
    }
    //line 5
    out << std::setw(5) << _NObs << std::endl;
    //line 6
    for (int i = 0; i < _NObs; ++i)
    {
        out << std::setw(4) << _iObs[i] << ' ';
    }
    out << std::endl;
    return true;
}

std::string ProfileObject::ToSqlStatement(const int gid)
{
    std::stringstream strbld;
    strbld << "INSERT INTO public.profile(gid, n, x, hnew, matnum, laynum,beta,ah,ak,ath";
    if (!_NS)
    {
        strbld << ") values";
        for (int i = 0; i < _NumNP; ++i)
        {
            strbld << "(" << gid << ","
                   << _n[i] << ","
                   << _x[i] << ","
                   << _hNew[i] << ","
                   << _MatNum[i] << ","
                   << _LayNum[i] << ","
                   << _Beta[i] << ","
                   << _Ah[i] << ","
                   << _Ak[i] << ","
                   << _Ath[i] << "),";
        }
    }
    else
    {
        strbld << ",";
        for (int i = 1; i < _NS; ++i)
        {
            strbld << "conc" << i << ",";
        }
        if (_sel->lEquil)
        {
            strbld << "conc" << _NS << ") values ";
        }
        else
        {
            strbld << "conc" << _NS << ",";
            for (int i = 1; i < _NS; ++i)
            {
                strbld << "sorb" << i << ",";
            }
            strbld << "sorb" << _NS << ") values ";
        }
        for (int i = 0; i < _NumNP; ++i)
        {
            strbld << "(" << gid << ","
                   << _n[i] << ","
                   << _x[i] << ","
                   << _hNew[i] << ","
                   << _MatNum[i] << ","
                   << _LayNum[i] << ","
                   << _Beta[i] << ","
                   << _Ah[i] << ","
                   << _Ak[i] << ","
                   << _Ath[i] << ",";
            double *pp = _Conc.get() + i * _NS;
            for (int j = 0; j < _NS - 1; ++j)
            {
                strbld << pp[j] << ",";
            }
            if (_sel->lEquil)
            {
                strbld << pp[_NS - 1] << "),";
            }
            else
            {
                strbld << pp[_NS - 1] << ",";
                pp = _Sorb.get() + i * _NS;
                for (int j = 0; j < _NS - 1; ++j)
                {
                    strbld << pp[j] << ",";
                }
                strbld << pp[_NS - 1] << "),";
            }
        }
    }
    std::string sql = strbld.str();
    sql.back() = ';';
    return sql;
}

bool ProfileObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in)
    {
        return false;
    }
    std::string line;
    //line 0--HYDRUS-1D version
    std::getline(in, line);
    Stringhelper qs(line);
    qs.trimmed();
    if (qs.startsWith("Pcp_File_Version="))
    {
        double version = std::stod(qs.str().substr(17));
        if (version < 4)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    //line 1
    std::getline(in, line);
    int nFix = std::stoi(line);
    //line 2
    for (int i = 0; i < nFix; ++i)
    {
        std::getline(in, line);
    }
    //line 3
    std::getline(in, line);
    std::vector<void *> pValue3 = {&_NumNP, &_NS, &_iTemp, &_iEquil};
    if (!ParseLine(line, "int,int,int,int", pValue3))
    {
        return false;
    }
    if (_NS != _sel->NS)
    {
        return false;
    }
    if (_sel->lChem)
    {
        if (_iTemp != 1)
        {
            return false;
        }
    }
    else
    {
        if (_iEquil != 1)
        {
            return false;
        }
    }
    if (_sel->lEquil)
    {
        if (_iEquil != 1)
        {
            return false;
        }
    }
    else
    {
        if (_iEquil != 0)
        {
            return false;
        }
    }
    _n = std::make_unique<int[]>(_NumNP);
    _x = std::make_unique<double[]>(_NumNP);
    _hNew = std::make_unique<double[]>(_NumNP);
    _MatNum = std::make_unique<int[]>(_NumNP);
    _LayNum = std::make_unique<int[]>(_NumNP);
    _Beta = std::make_unique<double[]>(_NumNP);
    _Ah = std::make_unique<double[]>(_NumNP);
    _Ak = std::make_unique<double[]>(_NumNP);
    _Ath = std::make_unique<double[]>(_NumNP);
    if (_sel->lChem)
    {
        _Conc = std::make_unique<double[]>(_NumNP * _NS);
    }
    if (!_sel->lEquil)
    {
        _Sorb = std::make_unique<double[]>(_NumNP * _NS);
    }
    //line 4
    if (_NS)
    {
        for (int i = 0; i < _NumNP; ++i)
        {
            std::getline(in, line);
            if (!ParseLine(line, i, _NS))
            {
                return false;
            }
        }
    }
    else
    {
        for (int i = 0; i < _NumNP; ++i)
        {
            std::getline(in, line);
            if (!ParseLine(line, i))
            {
                return false;
            }
        }
    }
    //line 5
    std::getline(in, line);
    _NObs = std::stoi(line);
    if (_NObs)
    {
        //line 6
        _iObs = std::make_unique<int[]>(_NObs);
        std::getline(in, line);
        Stringhelper h(line);
        auto sl = h.simplified().split(' ');
        try
        {
            for (int i = 0; i < _NObs; ++i)
            {
                _iObs[i] = std::stoi(sl[i]);
            }
        }
        catch (...)
        {
            return false;
        }
    }
    return true;
}

bool ProfileObject::open(int gid, pqxx::connection &qry)
{
    _NumNP = _sel->_NumNP;
    _n = std::make_unique<int[]>(_NumNP);
    _x = std::make_unique<double[]>(_NumNP);
    _hNew = std::make_unique<double[]>(_NumNP);
    _MatNum = std::make_unique<int[]>(_NumNP);
    _LayNum = std::make_unique<int[]>(_NumNP);
    _Beta = std::make_unique<double[]>(_NumNP);
    _Ah = std::make_unique<double[]>(_NumNP);
    _Ak = std::make_unique<double[]>(_NumNP);
    _Ath = std::make_unique<double[]>(_NumNP);
    _NS = _sel->NS;
    if (_NS)
    {
        _Conc = std::make_unique<double[]>(_NumNP * _NS);
        if (!_sel->lEquil)
        {
            _Sorb = std::make_unique<double[]>(_NumNP * _NS);
        }
    }
    _NObs = _sel->_NObs;
    if (_NObs)
    {
        _iObs = std::make_unique<int[]>(_NObs);
        for (int i = 0; i < _NObs; ++i)
        {
            _iObs[i] = _sel->_iObs[i];
        }
    }
    std::stringstream strbld;
    strbld << "select n, x, hnew, matnum, laynum,beta,ah,ak,ath";
    if (_NS)
    {
        strbld << ",";
        for (int i = 1; i < _NS; ++i)
        {
            strbld << "conc" << i << ",";
        }
        if (_sel->lEquil)
        {
            strbld << "conc" << _NS << " from profile where gid=" << gid << " order by n;";
        }
        else
        {
            strbld << "conc" << _NS << ",";
            for (int i = 1; i < _NS; ++i)
            {
                strbld << "sorb" << i << ",";
            }
            strbld << "sorb" << _NS << " from profile where gid=" << gid << " order by n;";
        }
        try
        {
            pqxx::work w(qry);
            pqxx::result r = w.exec(strbld.str());
            w.commit();
            if (r.empty())
            {
                return false;
            }
            int j = 0;
            for (auto it = r.begin(); it != r.end(); ++it)
            {
                _n[j] = it[0].as<int>();
                _x[j] = it[1].as<double>();
                _hNew[j] = it[2].as<double>();
                _MatNum[j] = it[3].as<int>();
                _LayNum[j] = it[4].as<int>();
                _Beta[j] = it[5].as<double>();
                _Ah[j] = it[6].as<double>();
                _Ak[j] = it[7].as<double>();
                _Ath[j] = it[8].as<double>();
                double *pp = _Conc.get() + j * _NS;
                for (int i = 0; i < _NS; ++i)
                {
                    pp[i] = it[9 + i].as<double>();
                }
                if (!_sel->lEquil)
                {
                    pp = _Sorb.get() + j * _NS;
                    for (int i = 0; i < _NS; ++i)
                    {
                        pp[i] = it[9 + _NS + i].as<double>();
                    }
                }
                j++;
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }
    }
    else
    {
        strbld << " from profile where gid=" << gid << " order by n;";
        try
        {
            pqxx::work w(qry);
            pqxx::result r = w.exec(strbld.str());
            w.commit();
            if (r.empty())
            {
                return false;
            }
            int j = 0;
            for (auto it = r.begin(); it != r.end(); ++it)
            {
                _n[j] = it[0].as<int>();
                _x[j] = it[1].as<double>();
                _hNew[j] = it[2].as<double>();
                _MatNum[j] = it[3].as<int>();
                _LayNum[j] = it[4].as<int>();
                _Beta[j] = it[5].as<double>();
                _Ah[j] = it[6].as<double>();
                _Ak[j] = it[7].as<double>();
                _Ath[j] = it[8].as<double>();
                j++;
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }
    }
    return true;
}

bool ProfileObject::ParseLine(const std::string &line, const std::string &lineformat, const std::vector<void *> &values)
{
    Stringhelper l(line);
    l.simplified();
    auto lst = l.split(' ');
    Stringhelper f(lineformat);
    auto format = f.split(',');
    if (lst.size() < format.size())
    {
        return false;
    }
    try
    {
        for (size_t i = 0; i < format.size(); ++i)
        {
            if (format[i] == "bool")
            {
                if (lst[i] == "f" || lst[i] == "F")
                {
                    *reinterpret_cast<bool *>(values[i]) = false;
                }
                else if (lst[i] == "t" || lst[i] == "T")
                {
                    *reinterpret_cast<bool *>(values[i]) = true;
                }
                else
                {
                    return false;
                }
            }
            else if (format[i] == "int")
            {
                *reinterpret_cast<int *>(values[i]) = std::stoi(lst[i]);
            }
            else if (format[i] == "double")
            {
                *reinterpret_cast<double *>(values[i]) = std::stod(lst[i]);
            }
            else
            {
                return false;
            }
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool ProfileObject::ParseLine(const std::string &line, const int lineindex)
{
    Stringhelper s(line);
    s.simplified();
    auto sl = s.split(' ');
    try
    {
        _n[lineindex] = std::stoi(sl[0]);
        _x[lineindex] = std::stod(sl[1]);
        _hNew[lineindex] = std::stod(sl[2]);
        _MatNum[lineindex] = std::stod(sl[3]);
        _LayNum[lineindex] = std::stod(sl[4]);
        _Beta[lineindex] = std::stod(sl[5]);
        _Ah[lineindex] = std::stod(sl[6]);
        _Ak[lineindex] = std::stod(sl[7]);
        _Ath[lineindex] = std::stod(sl[8]);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool ProfileObject::ParseLine(const std::string &line, const int lineindex, const int NS)
{
    Stringhelper s(line);
    s.simplified();
    auto sl = s.split(' ');
    try
    {
        _n[lineindex] = std::stoi(sl[0]);
        _x[lineindex] = std::stod(sl[1]);
        _hNew[lineindex] = std::stod(sl[2]);
        _MatNum[lineindex] = std::stod(sl[3]);
        _LayNum[lineindex] = std::stod(sl[4]);
        _Beta[lineindex] = std::stod(sl[5]);
        _Ah[lineindex] = std::stod(sl[6]);
        _Ak[lineindex] = std::stod(sl[7]);
        _Ath[lineindex] = std::stod(sl[8]);
        double *pp = _Conc.get() + lineindex * _NS;
        for (int i = 0; i < _NS; ++i)
        {
            pp[i] = std::stod(sl[10 + i]);
        }
        if (!_sel->lEquil)
        {
            pp = _Sorb.get() + lineindex * _NS;
            for (int i = 0; i < NS; ++i)
            {
                pp[i] = std::stod(sl[10 + NS + i]);
            }
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void ProfileObject::Initial()
{
}

void ProfileObject::SaveLine(std::ostream &out, const int lineindex)
{
    auto precision = out.precision();
    out << std::setw(4) << _n[lineindex];
    out << std::fixed << std::setprecision(6);
    out << std::setw(15) << fwzformat::SE3 << _x[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _hNew[lineindex];
    out.unsetf(std::ios_base::fixed);
    out << std::setw(4) << _MatNum[lineindex];
    out << std::setw(4) << _LayNum[lineindex];
    out << std::fixed << std::setprecision(6);
    out << std::setw(15) << fwzformat::SE3 << _Beta[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _Ah[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _Ak[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _Ath[lineindex];
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    out << std::endl;
}

void ProfileObject::SaveLine(std::ostream &out, const int lineindex, const int NS)
{
    auto precision = out.precision();
    out << std::setw(4) << _n[lineindex];
    out << std::fixed << std::setprecision(6);
    out << std::setw(15) << fwzformat::SE3 << _x[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _hNew[lineindex];
    out.unsetf(std::ios_base::fixed);
    out << std::setw(4) << _MatNum[lineindex];
    out << std::setw(4) << _LayNum[lineindex];
    out << std::fixed << std::setprecision(6);
    out << std::setw(15) << fwzformat::SE3 << _Beta[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _Ah[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _Ak[lineindex];
    out << std::setw(15) << fwzformat::SE3 << _Ath[lineindex];
    out << std::setw(15) << fwzformat::SE3 << 20.0;
    double *pp = _Conc.get() + lineindex * NS;
    for (int i = 0; i < NS; ++i)
    {
        out << std::setw(15) << fwzformat::SE3 << pp[i];
    }
    if (!_sel->lEquil)
    {
        pp = _Sorb.get() + lineindex * NS;
        for (int i = 0; i < NS; ++i)
        {
            out << std::setw(15) << fwzformat::SE3 << pp[i];
        }
    }
    out.unsetf(std::ios_base::fixed);
    out.precision(precision);
    out << std::endl;
}
