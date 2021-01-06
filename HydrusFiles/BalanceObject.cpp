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
#include <limits>
#include "Stringhelper.h"
#include "HydrusParameterFilesManager.h"
#include "BalanceObject.h"
#include "FFmt.h"
#include "Stringhelper.h"

BalanceObject::BalanceRecord::BalanceRecord(BalanceObject &parent, std::list<std::string> &part)
{
    _Vn = _V1 = std::numeric_limits<double>::max();
    _wBalT = std::numeric_limits<double>::max();
    _wBalR = 9999;
    for (auto it = part.begin(); it != part.end(); ++it)
    {
        Stringhelper s(*it);
        s.simplified();
        if (s.startsWith("Time"))
        {
            unsigned int nsize = it->size();
            _Time = std::stod(it->substr(nsize - 14, 14));
            parent._isValid = true;
        }
        else if (s.startsWith("Area") || s.startsWith("Length"))
        {
            parent._isValid = ParseArea(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("W-volume"))
        {
            parent._isValid = ParseW_volumn(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("In-flow"))
        {
            parent._isValid = ParseInflow(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("h Mean"))
        {
            parent._isValid = ParsehMean(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("ConcVol"))
        {
            if (!_ConVol && parent._NS)
            {
                _ConVol = std::make_unique<double[]>(parent._NS);
                _ConSub = std::make_unique<double[]>(parent._NLayer * parent._NS);
            }
            parent._isValid = ParseConcVol(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("ConcVolIm"))
        {
            if (!_ConVolIm && parent._NS)
            {
                _ConVolIm = std::make_unique<double[]>(parent._NS);
                _ConSubIm = std::make_unique<double[]>(parent._NLayer * parent._NS);
            }
            parent._isValid = ParseConcVolIm(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("SorbVolIm"))
        {
            if (!_ConVolIm && parent._NS)
            {
                _ConVolIm = std::make_unique<double[]>(parent._NS);
                _ConSubIm = std::make_unique<double[]>(parent._NLayer * parent._NS);
            }
            parent._isValid = ParseSorbVolIm(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("cMean"))
        {
            if (!_cTot && parent._NS)
            {
                _cTot = std::make_unique<double[]>(parent._NS);
                _cMean = std::make_unique<double[]>(parent._NLayer * parent._NS);
            }
            parent._isValid = ParseCMean(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("cMeanIm"))
        {
            if (!_cMeanIm && parent._NS)
            {
                _cTotIm = std::make_unique<double[]>(parent._NS);
                _cMeanIm = std::make_unique<double[]>(parent._NLayer * parent._NS);
            }
            parent._isValid = ParseCMeanIM(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("sMeanIm"))
        {
            if (!_cMeanIm && parent._NS)
            {
                _cTotIm = std::make_unique<double[]>(parent._NS);
                _cMeanIm = std::make_unique<double[]>(parent._NLayer * parent._NS);
            }
            parent._isValid = ParseSMeanIm(it->c_str(), parent._NLayer);
        }
        else if (s.startsWith("Top Flux"))
        {
            parent._isValid = ParseTopFlux(it->c_str());
        }
        else if (s.startsWith("Bot Flux"))
        {
            parent._isValid = ParseBotFlux(it->c_str());
        }
        else if (s.startsWith("WatBalT"))
        {
            parent._isValid = ParseWatBalT(it->c_str());
        }
        else if (s.startsWith("WatBalR"))
        {
            parent._isValid = ParseWatBalR(it->c_str());
        }
        else if (s.startsWith("CncBalT"))
        {
            if (parent._NS && !_cBalT)
            {
                _cBalT = std::make_unique<double[]>(parent._NS);
            }
            parent._isValid = ParseCncBalT(it->c_str());
        }
        else if (s.startsWith("CncBalR"))
        {
            if (parent._NS && !_cBalR)
            {
                _cBalR = std::make_unique<double[]>(parent._NS);
                for (int i = 0; i < parent._NS; ++i)
                {
                    _cBalR[i] = 9999.;
                }
            }
            parent._isValid = ParseCncBalR(it->c_str());
        }
        else
        {
            parent._isValid = false;
        }
        if (!parent._isValid)
        {
            break;
        }
    }
}

BalanceObject::BalanceRecord::BalanceRecord(const int NLayer, const int NS)
{
    _Vn = std::numeric_limits<double>::max();
    _V1 = std::numeric_limits<double>::max();
    _wBalT = std::numeric_limits<double>::max();
    _wBalR = 9999;
    _Area = std::make_unique<double[]>(NLayer);
    _hMean = std::make_unique<double[]>(NLayer);
    _SubCha = std::make_unique<double[]>(NLayer);
    _SubVol = std::make_unique<double[]>(NLayer);
    if (NS)
    {
        _ConVol = std::make_unique<double[]>(NS);
        _ConSub = std::make_unique<double[]>(NLayer * NS);
        _ConVolIm = std::make_unique<double[]>(NS);
        _ConSubIm = std::make_unique<double[]>(NLayer * NS);
        _cTot = std::make_unique<double[]>(NS);
        _cMean = std::make_unique<double[]>(NLayer * NS);
        _cTotIm = std::make_unique<double[]>(NS);
        _cMeanIm = std::make_unique<double[]>(NLayer * NS);
        _cBalT = std::make_unique<double[]>(NS);
        _cBalR = std::make_unique<double[]>(NS);
        for (int i = 0; i < NS; ++i)
        {
            _cBalR[i] = 9999.0;
        }
    }
}

bool BalanceObject::BalanceRecord::ParseArea(const char *pLine, const int NLayer)
{
    try
    {
        if (!_Area)
        {
            _Area = std::make_unique<double[]>(NLayer);
        }
        char *p = const_cast<char *>(pLine) + 19;
        std::string s(p, 13);
        _ATot = std::stod(s);
        p += 13;
        for (int i = 0; i < NLayer; ++i)
        {
            _Area[i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseW_volumn(const char *pLine, const int NLayer)
{
    try
    {
        if (!_SubVol)
        {
            _SubVol = std::make_unique<double[]>(NLayer);
        }
        char *p = const_cast<char *>(pLine) + 19;
        std::string s(p, 13);
        _Volume = std::stod(s);
        p += 13;
        for (int i = 0; i < NLayer; ++i)
        {
            _SubVol[i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseInflow(const char *pLine, const int NLayer)
{
    try
    {
        if (!_SubCha)
        {
            _SubCha = std::make_unique<double[]>(NLayer);
        }
        char *p = const_cast<char *>(pLine) + 19;
        std::string s(p, 13);
        _Change = std::stod(s);
        p += 13;
        for (int i = 0; i < NLayer; ++i)
        {
            _SubCha[i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParsehMean(const char *pLine, const int NLayer)
{
    try
    {
        if (!_hMean)
        {
            _hMean = std::make_unique<double[]>(NLayer);
        }
        char *p = const_cast<char *>(pLine) + 19;
        std::string s(p, 13);
        _hTot = std::stod(s);
        p += 13;
        for (int i = 0; i < NLayer; ++i)
        {
            _hMean[i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseConcVol(const char *pLine, const int NLayer)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        std::string s(p, 13);
        _ConVol[idx] = std::stod(s);
        p += 13;
        int idx2 = idx * NLayer;
        for (int i = 0; i < NLayer; ++i)
        {
            _ConSub[idx2 + i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseConcVolIm(const char *pLine, const int NLayer)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        std::string s(p, 13);
        _ConVolIm[idx] = std::stod(s);
        p += 13;
        int idx2 = idx * NLayer;
        for (int i = 0; i < NLayer; ++i)
        {
            _ConSubIm[idx2 + i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseSorbVolIm(const char *pLine, const int NLayer)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        std::string s(p, 13);
        _ConVolIm[idx] = std::stod(s);
        p += 13;
        int idx2 = idx * NLayer;
        for (int i = 0; i < NLayer; ++i)
        {
            _ConSubIm[idx2 + i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseSMeanIm(const char *pLine, const int NLayer)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        std::string s(p, 13);
        _cTotIm[idx] = std::stod(s);
        p += 13;
        int idx2 = idx * NLayer;
        for (int i = 0; i < NLayer; ++i)
        {
            _cMeanIm[idx2 + i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseCMean(const char *pLine, const int NLayer)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        std::string s(p, 13);
        _cTot[idx] = std::stod(s);
        p += 13;
        int idx2 = idx * NLayer;
        for (int i = 0; i < NLayer; ++i)
        {
            _cMean[idx2 + i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseCMeanIM(const char *pLine, const int NLayer)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        std::string s(p, 13);
        _cTotIm[idx] = std::stod(s);
        p += 13;
        int idx2 = idx * NLayer;
        for (int i = 0; i < NLayer; ++i)
        {
            _cMeanIm[idx2 + i] = std::stod(std::string(p, 13));
            p += 13;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseTopFlux(const char *pLine)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 19;
        _Vn = std::stod(std::string(p, 13));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseBotFlux(const char *pLine)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 19;
        _V1 = std::stod(std::string(p, 13));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseWatBalT(const char *pLine)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 19;
        _wBalT = std::stod(std::string(p, 13));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseWatBalR(const char *pLine)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 19;
        _wBalR = std::stod(std::string(p, 13));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseCncBalT(const char *pLine)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        _cBalT[idx] = std::stod(std::string(p, 13));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BalanceObject::BalanceRecord::ParseCncBalR(const char *pLine)
{
    try
    {
        char *p = const_cast<char *>(pLine) + 17;
        int idx = *p - '1';
        p += 2;
        _cBalR[idx] = std::stod(std::string(p, 13));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

BalanceObject::BalanceObject(const std::string &filename, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _NLayer = _parent->NumofLayer();
    _NS = _parent->NumofSolute();
    _CalTm = _parent->CalCulationTime();
    _isValid = open(filename);
}

BalanceObject::BalanceObject(int gid, pqxx::connection &qry, HydrusParameterFilesManager *parent)
{
    _parent = parent;
    _NLayer = _parent->NumofLayer();
    _NS = _parent->NumofSolute();
    _CalTm = _parent->CalCulationTime();
    _isValid = open(gid, qry);
}

BalanceObject::~BalanceObject()
{
}

bool BalanceObject::Save(const std::string &path)
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
    std::ofstream out((std::filesystem::absolute(p) / "Balance.out").string());
    if (!out)
    {
        return false;
    }
    WriteHead(out);
    int i = 0;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        WriteSection(out, **it, i++);
    }
    WriteEnd(out);
    out.close();
    return true;
}

bool BalanceObject::Save(std::ostream &out)
{
    if (!_isValid || !out)
    {
        return false;
    }
    WriteHead(out);
    int i = 0;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        WriteSection(out, **it, i++);
    }
    WriteEnd(out);
    return true;
}

void BalanceObject::WriteHead(std::ostream &out)
{
    out << " ******* Program HYDRUS" << std::endl;
    out << std::left;
    if (_parent->HeadLineCount() == 4)
    {
        out << " ******* " << std::setw(72) << _parent->HeadContent() << std::endl;
    }
    else
    {
        out << " ******* " << std::endl
            << ' ' << std::setw(72) << _parent->HeadContent() << std::endl;
    }
    out << std::right;
    //Hydrus output head format
    //format(' Date: ',i3,'.',i2,'.','    Time: ',i3,':',i2,':',i2)
    out << " Date: " << std::setw(3) << _parent->Day() << '.'
        << std::setw(2) << _parent->Mon() << '.'
        << "    Time: " << std::setw(3) << _parent->Hour() << ':'
        << std::setw(2) << _parent->Mints() << ':'
        << std::setw(2) << _parent->Secs() << std::endl;
    out << std::left;
    out << " Units: L = " << std::setw(5) << _parent->LUnit()
        << ", T = " << std::setw(5) << _parent->TUnit()
        << ", M = " << std::setw(5) << _parent->MUnit() << std::endl;
    out << std::right;
}
//    110   format(
//                !    /'----------------------------------------------------------'/
//                !        ' Time       [T]',f14.4/
//                !     '----------------------------------------------------------')
//    120   format( ' Sub-region num.               ',9(I7,6x))
//    130   format(
//                !     '----------------------------------------------------------')
//    140   format( ' Length   [L]      ',e13.5,9e13.5)
//    150   format( ' W-volume [L]      ',e13.5,9e13.5)
//    160   format( ' In-flow  [L/T]    ',e13.5,9e13.5)
//    170   format( ' h Mean   [L]      ',e13.5,9e13.5)
//    200   format( ' ConcVol  [M/L2] ',i1,1x,e13.5,10e13.5)
//    210   format( ' cMean    [M/L3] ',i1,1x,e13.5,10e13.5)
//    202   format( ' SorbVolIm[M/L2] ',i1,1x,e13.5,10e13.5)
//    212   format( ' sMeanIm  [-]    ',i1,1x,e13.5,10e13.5)
//    220   format( ' Top Flux [L/T]    ',e13.5/
//         !        ' Bot Flux [L/T]    ',e13.5)
//    230   format( ' WatBalT  [L]      ',e13.5)
//    240   format( ' WatBalR  [%]      ',f13.3)
//    250   format( ' CncBalT  [M]    ',i1,1x,e13.5)
//    260   format( ' CncBalR  [%]    ',i1,1x,f13.3)
void BalanceObject::WriteSection(std::ostream &out, BalanceRecord &rec, const int i)
{
    out << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    out << " Time       [T]" << std::setw(14) << std::setprecision(4) << std::fixed << rec._Time << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    out << " Sub-region num.               ";
    for (int i = 1; i < _NLayer; ++i)
    {
        out << std::setw(7) << i << std::setw(6) << ' ';
    }
    out << std::setw(7) << _NLayer << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    out << " Length   [L]      " << std::setprecision(5) << std::setw(13) << fwzformat::fortranE2 << rec._ATot;
    for (int i = 0; i < _NLayer; ++i)
    {
        out << std::setw(13) << fwzformat::fortranE2 << rec._Area[i];
    }
    out << std::endl;
    if (rec._SubVol)
    {
        out << " W-volume [L]      " << std::setw(13) << fwzformat::fortranE2 << rec._Volume;
        for (int i = 0; i < _NLayer; ++i)
        {
            out << std::setw(13) << fwzformat::fortranE2 << rec._SubVol[i];
        }
        out << std::endl;
        out << " In-flow  [L/T]    " << std::setw(13) << fwzformat::fortranE2 << rec._Change;
        for (int i = 0; i < _NLayer; ++i)
        {
            out << std::setw(13) << fwzformat::fortranE2 << rec._SubCha[i];
        }
        out << std::endl;
        out << " h Mean   [L]      " << std::setw(13) << fwzformat::fortranE2 << rec._hTot;
        for (int i = 0; i < _NLayer; ++i)
        {
            out << std::setw(13) << fwzformat::fortranE2 << rec._hMean[i];
        }
        out << std::endl;
    }
    for (int i = 0; i < _NS; ++i)
    {
        int idx = i * _NLayer;
        out << " ConcVol  [M/L2] " << (i + 1) << std::setw(14) << fwzformat::fortranE2 << rec._ConVol[i];
        for (int j = 0; j < _NLayer; ++j)
        {
            out << std::setw(13) << fwzformat::fortranE2 << rec._ConSub[idx + j];
        }
        out << std::endl;
        out << " cMean    [M/L3] " << (i + 1) << std::setw(14) << fwzformat::fortranE2 << rec._cTot[i];
        for (int j = 0; j < _NLayer; ++j)
        {
            out << std::setw(13) << fwzformat::fortranE2 << rec._cMean[idx + j];
        }
        out << std::endl;
        if (rec._ConVolIm)
        {
            out << " SorbVolIm[M/L2] " << (i + 1) << std::setw(14) << fwzformat::fortranE2 << rec._ConVolIm[i];
            for (int j = 0; j < _NLayer; ++j)
            {
                out << std::setw(13) << fwzformat::fortranE2 << rec._ConSubIm[idx + j];
            }
            out << std::endl;
            out << " sMeanIm  [-]    " << (i + 1) << std::setw(14) << fwzformat::fortranE2 << rec._cTotIm[i];
            for (int j = 0; j < _NLayer; ++j)
            {
                out << std::setw(13) << fwzformat::fortranE2 << rec._cMeanIm[idx + j];
            }
            out << std::endl;
        }
    }
    if (std::abs(rec._Vn - std::numeric_limits<double>::max()) > std::numeric_limits<double>::epsilon())
    {
        out << " Top Flux [L/T]    " << std::setprecision(5) << std::setw(13) << fwzformat::fortranE2 << rec._Vn << std::endl;
    }
    if (std::abs(rec._V1 - std::numeric_limits<double>::max()) > std::numeric_limits<double>::epsilon())
    {
        out << " Bot Flux [L/T]    " << std::setprecision(5) << std::setw(13) << fwzformat::fortranE2 << rec._V1 << std::endl;
    }
    if (i)
    {
        if (std::abs(rec._wBalT - std::numeric_limits<double>::max()) > std::numeric_limits<double>::epsilon())
        {
            out << " WatBalT  [L]      " << std::setprecision(5) << std::setw(13) << fwzformat::fortranE2 << rec._wBalT << std::endl;
            if (std::abs(rec._wBalR - 9999) > std::numeric_limits<double>::epsilon())
            {
                out << " WatBalR  [%]      " << std::setprecision(3) << std::setw(13) << rec._wBalR << std::endl;
            }
        }
        for (int i = 0; i < _NS; ++i)
        {
            out << " CncBalT  [M]    " << (i + 1) << std::setprecision(5) << std::setw(14) << fwzformat::fortranE2 << rec._cBalT[i] << std::endl;
            if (std::abs(rec._cBalR[i] - 9999) > std::numeric_limits<double>::epsilon())
            {
                out << " CncBalR  [%]    " << (i + 1) << std::setprecision(3) << std::setw(14) << rec._cBalR[i] << std::endl;
            }
        }
    }
    out << "----------------------------------------------------------" << std::endl;
}

void BalanceObject::WriteEnd(std::ostream &out)
{
    out << std::endl;
    out << "Calculation time [sec]" << std::setw(20) << std::fixed << std::setprecision(12) << _CalTm << std::endl;
}

std::string BalanceObject::ToSqlStatementPart1(const int gid)
{
    if (_Recs.empty())
    {
        return "";
    }
    std::stringstream strbld;
    strbld << "INSERT INTO public.balancetotal("
              "gid, tm, length, volumn, inflow, hmean, concvol1, concvol2, concvol3,"
              "concvol4, concvol5, concvol6, concvol7, concvol8, concvol9, concvol10,"
              "sorbvolim1, sorbvolim2, sorbvolim3, sorbvolim4, sorbvolim5, sorbvolim6,"
              "sorbvolim7, sorbvolim8, sorbvolim9, sorbvolim10, cmean1, cmean2,"
              "cmean3, cmean4, cmean5, cmean6, cmean7, cmean8, cmean9, cmean10,"
              "smeanim1, smeanim2, smeanim3, smeanim4, smeanim5, smeanim6, smeanim7,"
              "smeanim8, smeanim9, smeanim10, topflux, botflux, watbalt, watbalr,"
              "cncbalt1, cncbalt2, cncbalt3, cncbalt4, cncbalt5, cncbalt6, cncbalt7,"
              "cncbalt8, cncbalt9, cncbalt10, cncbalr1, cncbalr2, cncbalr3,"
              "cncbalr4, cncbalr5, cncbalr6, cncbalr7, cncbalr8, cncbalr9, cncbalr10) values";
    Stringhelper stemplate("(%1, %2, %3, %4, %5, %6, %7, %8, %9,"
                           "%10, %11, %12, %13, %14, %15, %16,"
                           "%17, %18, %19, %20, %21, %22,"
                           "%23, %24, %25, %26, %27, %28,"
                           "%29, %30, %31, %32, %33, %34, %35, %36,"
                           "%37, %38, %39, %40, %41, %42, %43,"
                           "%44, %45, %46, %47, %48, %49, %50,"
                           "%51, %52, %53, %54, %55, %56, %57,"
                           "%58, %59, %60, %61, %62, %63,"
                           "%64, %65, %66, %67, %68, %69, %70),");
    int j = 0;
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        Stringhelper s = stemplate;
        BalanceRecord &rec = **it;
        s = s.arg(gid).arg(rec._Time).arg(rec._ATot);
        if (rec._SubVol)
        {
            s = s.arg(rec._Volume).arg(rec._Change).arg(rec._hTot);
        }
        else
        {
            s = s.arg("null").arg("null").arg("null");
        }
        for (int i = 0; i < 10; ++i)
        {
            if (i < _NS)
            {
                s = s.arg(rec._ConVol[i]);
            }
            else
            {
                s = s.arg("null");
            }
        }
        for (int i = 0; i < 10; ++i)
        {
            if (i < _NS && rec._ConVolIm)
            {
                s = s.arg(rec._ConVolIm[i]);
            }
            else
            {
                s = s.arg("null");
            }
        }
        for (int i = 0; i < 10; ++i)
        {
            if (i < _NS)
            {
                s = s.arg(rec._cTot[i]);
            }
            else
            {
                s = s.arg("null");
            }
        }
        for (int i = 0; i < 10; ++i)
        {
            if (i < _NS && rec._cTotIm)
            {
                s = s.arg(rec._cTotIm[i]);
            }
            else
            {
                s = s.arg("null");
            }
        }
        if (std::abs(rec._Vn - std::numeric_limits<double>::max()) > std::numeric_limits<double>::epsilon())
        {
            s = s.arg(rec._Vn);
        }
        else
        {
            s = s.arg("null");
        }
        if (std::abs(rec._V1 - std::numeric_limits<double>::max()) > std::numeric_limits<double>::epsilon())
        {
            s = s.arg(rec._V1);
        }
        else
        {
            s = s.arg("null");
        }
        if (j)
        {
            if (std::abs(rec._wBalT - std::numeric_limits<double>::max()) > std::numeric_limits<double>::epsilon())
            {
                s = s.arg(rec._wBalT);
                if (std::abs(rec._wBalR - 9999) > std::numeric_limits<double>::epsilon())
                {
                    s = s.arg(rec._wBalR);
                }
                else
                {
                    s = s.arg("null");
                }
            }
            else
            {
                s = s.arg("null").arg("null");
            }
            for (int i = 0; i < 10; ++i)
            {
                if (i < _NS)
                {
                    s = s.arg(rec._cBalT[i]);
                }
                else
                {
                    s = s.arg("null");
                }
            }
            for (int i = 0; i < 10; ++i)
            {
                if (i < _NS && std::abs(rec._cBalR[i] - 9999) > std::numeric_limits<double>::epsilon())
                {
                    s = s.arg(rec._cBalR[i]);
                }
                else
                {
                    s = s.arg("null");
                }
            }
        }
        else
        {
            for (int i = 0; i < 22; ++i)
            {
                s = s.arg("null");
            }
        }
        j++;
        strbld << s.str();
    }
    std::string str = strbld.str();
    str.back() = ';';
    return str;
}

std::string BalanceObject::ToSqlStatementPart2(const int gid)
{
    if (_Recs.empty())
    {
        return "";
    }
    std::stringstream strbld;
    strbld << "insert into public.balancelayer("
              "gid, tm, layer, length, volumn, inflow, hmean, concvol1, concvol2,"
              "concvol3, concvol4, concvol5, concvol6, concvol7, concvol8, concvol9,"
              "concvol10, cmean1, cmean2, cmean3, cmean4, cmean5, cmean6, cmean7,"
              "cmean8, cmean9, cmean10, sorbvolim1, sorbvolim2, sorbvolim3,"
              "sorbvolim4, sorbvolim5, sorbvolim6, sorbvolim7, sorbvolim8, sorbvolim9,"
              "sorbvolim10, smeanim1, smeanim2, smeanim3, smeanim4, smeanim5,"
              "smeanim6, smeanim7, smeanim8, smeanim9, smeanim10) values";
    Stringhelper stemplate("(%1, %2, %3, %4, %5, %6, %7, %8, %9,"
                           "%10, %11, %12, %13, %14, %15, %16,"
                           "%17, %18, %19, %20, %21, %22,"
                           "%23, %24, %25, %26, %27, %28,"
                           "%29, %30, %31, %32, %33, %34, %35, %36,"
                           "%37, %38, %39, %40, %41, %42, %43,"
                           "%44, %45, %46, %47),");
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        BalanceRecord &rec = **it;
        for (int i = 0; i < _NLayer; ++i)
        {
            Stringhelper s = stemplate;
            s = s.arg(gid).arg(rec._Time).arg(i + 1).arg(rec._Area[i]);
            if (rec._SubVol)
            {
                s = s.arg(rec._SubVol[i]).arg(rec._SubCha[i]).arg(rec._hMean[i]);
            }
            else
            {
                s = s.arg("null").arg("null").arg("null");
            }
            for (int j = 0; j < 10; ++j)
            {
                if (j < _NS)
                {
                    s = s.arg(rec._ConSub[j * _NLayer + i]);
                }
                else
                {
                    s = s.arg("null");
                }
            }
            for (int j = 0; j < 10; ++j)
            {
                if (j < _NS)
                {
                    s = s.arg(rec._cMean[j * _NLayer + i]);
                }
                else
                {
                    s = s.arg("null");
                }
            }
            for (int j = 0; j < 10; ++j)
            {
                if (j < _NS && rec._ConSubIm)
                {
                    s = s.arg(rec._ConSubIm[j * _NLayer + i]);
                }
                else
                {
                    s = s.arg("null");
                }
            }
            for (int j = 0; j < 10; ++j)
            {
                if (j < _NS && rec._cMeanIm)
                {
                    s = s.arg(rec._cMeanIm[j]);
                }
                else
                {
                    s = s.arg("null");
                }
            }
            strbld << s.str();
        }
    }
    std::string str = strbld.str();
    str.back() = ';';
    return str;
}

bool BalanceObject::QueryTotalTable(int gid, pqxx::connection &qry)
{
    std::stringstream strbld;
    strbld << "SELECT tm, length, volumn, inflow, hmean,";
    std::string fields[] = {"concvol", "cmean", "cncbalt", "sorbvolim", "smeanim", "cncbalr"};
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 1; j <= _NS; ++j)
        {
            strbld << fields[i] << j << ",";
        }
    }
    strbld << "topflux, botflux, watbalt, watbalr from balancetotal where gid=" << gid << " order by tm;";
    try
    {
        pqxx::work w(qry);
        pqxx::result r = w.exec(strbld.str());
        w.commit();
        if (r.empty())
        {
            return false;
        }
        std::unique_ptr<BalanceRecord> pRec;
        int j = 0;
        for (auto it = r.begin(); it != r.end(); ++it)
        {
            pRec = std::make_unique<BalanceRecord>(_NLayer, _NS);
            pRec->_Time = it[0].as<double>();
            pRec->_ATot = it[1].as<double>();
            if (it[2].is_null())
            {
                pRec->_SubVol = nullptr;
                pRec->_SubCha = nullptr;
                pRec->_hMean = nullptr;
            }
            else
            {
                pRec->_Volume = it[2].as<double>();
                pRec->_Change = it[3].as<double>();
                pRec->_hTot = it[4].as<double>();
            }
            for (int i = 0; i < _NS; ++i)
            {
                pRec->_ConVol[i] = it[5 + i].as<double>();
            }
            for (int i = 0; i < _NS; ++i)
            {
                pRec->_cTot[i] = it[5 + _NS + i].as<double>();
            }
            if (j)
            {
                for (int i = 0; i < _NS; ++i)
                {
                    pRec->_cBalT[i] = it[5 + 2 * _NS + i].as<double>();
                }
            }
            if (it[5 + 3 * _NS].is_null())
            {
                pRec->_ConVolIm = nullptr;
                pRec->_ConSubIm = nullptr;
            }
            else
            {
                for (int i = 0; i < _NS; ++i)
                {
                    pRec->_ConVolIm[i] = it[5 + 3 * _NS + i].as<double>();
                }
            }
            if (it[5 + 4 * _NS].is_null())
            {
                pRec->_cTotIm = nullptr;
                pRec->_cMeanIm = nullptr;
            }
            else
            {
                for (int i = 0; i < _NS; ++i)
                {
                    pRec->_cTotIm[i] = it[5 + 4 * _NS + i].as<double>();
                }
            }
            if (j)
            {
                for (int i = 0; i < _NS; ++i)
                {
                    if (!it[5 + 5 * _NS + i].is_null())
                    {
                        pRec->_cBalR[i] = it[5 + 5 * _NS + i].as<double>();
                    }
                }
            }
            if (!it[5 + 6 * _NS].is_null())
            {
                pRec->_Vn = it[5 + 6 * _NS].as<double>();
            }
            if (!it[6 + 6 * _NS].is_null())
            {
                pRec->_V1 = it[6 + 6 * _NS].as<double>();
            }
            if (j)
            {
                if (!it[7 + 6 * _NS].is_null())
                {
                    pRec->_wBalT = it[7 + 6 * _NS].as<double>();
                }
                if (!it[8 + 6 * _NS].is_null())
                {
                    pRec->_wBalR = it[8 + 6 * _NS].as<double>();
                }
            }
            _Recs.push_back(std::move(pRec));
            j++;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool BalanceObject::QueryLayerTable(int gid, pqxx::connection &qry)
{
    std::stringstream strbld;
    strbld << "SELECT tm, layer, length, volumn, inflow, hmean";
    std::string fields[] = {"concvol", "cmean", "sorbvolim", "smeanim"};
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 1; j <= _NS; ++j)
        {
            strbld << "," << fields[i] << j;
        }
    }
    strbld << " from balancelayer where gid=" << gid << " order by tm,layer;";
    try
    {
        pqxx::work w(qry);
        pqxx::result r = w.exec(strbld.str());
        w.commit();
        if (r.empty())
        {
            return false;
        }
        BalanceRecord *pRec = nullptr;
        for (auto it = r.begin(); it != r.end(); ++it)
        {
            double tm = it[0].as<double>();
            if (!pRec || std::abs(pRec->_Time - tm) > std::numeric_limits<double>::epsilon())
            {
                pRec = GetRecord(tm);
                if (!pRec)
                {
                    return false;
                }
            }
            int layeridx = it[1].as<int>() - 1;
            pRec->_Area[layeridx] = it[2].as<double>();
            if (pRec->_SubVol)
            {
                pRec->_SubVol[layeridx] = it[3].as<double>();
                pRec->_SubCha[layeridx] = it[4].as<double>();
                pRec->_hMean[layeridx] = it[5].as<double>();
            }
            for (int i = 0; i < _NS; ++i)
            {
                pRec->_ConSub[i * _NLayer + layeridx] = it[6 + i].as<double>();
            }
            for (int i = 0; i < _NS; ++i)
            {
                pRec->_cMean[i * _NLayer + layeridx] = it[6 + _NS + i].as<double>();
            }

            if (pRec->_ConSubIm)
            {
                for (int i = 0; i < _NS; ++i)
                {
                    pRec->_ConSubIm[i * _NLayer + layeridx] = it[6 + 2 * _NS + i].as<double>();
                }
            }
            if (pRec->_cMeanIm)
            {
                for (int i = 0; i < _NS; ++i)
                {
                    pRec->_cMeanIm[i * _NLayer + layeridx] = it[6 + 3 * _NS + i].as<double>();
                }
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

BalanceObject::BalanceRecord *BalanceObject::GetRecord(double tm)
{
    for (auto it = _Recs.begin(); it != _Recs.end(); ++it)
    {
        if (std::abs((*it)->_Time - tm) < std::numeric_limits<double>::epsilon())
        {
            return (*it).get();
        }
    }
    return nullptr;
}

std::string BalanceObject::ToSqlStatement(const int gid)
{
    std::stringstream strbld;
    std::string s = Stringhelper("update selector set caltm=%1 where gid=%2;").arg(_CalTm).arg(gid).str();
    strbld << ToSqlStatementPart1(gid) << ToSqlStatementPart2(gid) << s;
    return strbld.str();
}

bool BalanceObject::open(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in)
    {
        return false;
    }

    int i = 0;
    //ignore the head lines
    std::string line;
    while (i++ < _parent->HeadLineCount())
    {
        std::getline(in, line);
    }
    std::unique_ptr<BalanceRecord> pRec;
    std::list<std::string> _Section;
    while (std::getline(in, line)) //ignore the blank line
    {
        std::getline(in, line);
        if (line.find("Calculation time [sec]") != std::string::npos)
        {
            auto p = line.find(']');
            _CalTm = std::stod(line.substr(p + 1));
            break;
        }
        //Get Time infomation
        std::getline(in, line);
        if (line.back() == '\r')
        {
            line.pop_back();
        }
        _Section.push_back(line);
        std::getline(in, line);
        std::getline(in, line);
        std::getline(in, line);
        while (true)
        {
            std::getline(in, line);
            if (line.find("----------------------------------------------------------") != std::string::npos)
            {
                pRec = std::make_unique<BalanceRecord>(*this, _Section);
                if (_isValid)
                {
                    _Recs.push_back(std::move(pRec));
                    _Section.clear();
                    break;
                }
                return false;
            }
            if (line.back() == '\r')
            {
                line.pop_back();
            }
            _Section.push_back(line);
        }
    }
    return true;
}

bool BalanceObject::open(int gid, pqxx::connection &qry)
{
    return QueryTotalTable(gid, qry) && QueryLayerTable(gid, qry);
}
