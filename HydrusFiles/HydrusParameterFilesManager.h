
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

#ifndef HYDRUSPARAMETERFILESMANAGER_H
#define HYDRUSPARAMETERFILESMANAGER_H
#include <string>
#include <map>
#include <memory>
#include "SelectorObject.h"
#include "AtmosphObject.h"
#include "ProfileObject.h"
#include "ALevelObject.h"
#include "TLevelObject.h"
#include "NodInfoObject.h"
#include "ObsNodeObject.h"
#include "BalanceObject.h"
#include "SoluteObject.h"

class QSqlQuery;

class HydrusParameterFilesManager
{
public:
    friend class SelectorObject;
    friend class TLevelObject;
    HydrusParameterFilesManager(int gid,const std::string& path,QSqlQuery& qry);

    bool HasErr() const
    {
        return _err;
    }
    std::string ErrMessage() const
    {
        return _errMessage;
    }
    int NumofLayer() const
    {
        return _NLayer;
    }
    int NumofSolute() const
    {
        return _NS;
    }
    int HeadLineCount() const
    {
        return _HeadLine;
    }
    std::string HeadContent() const
    {
        return _Hed;
    }
    int Mon() const
    {
        return _imonth;
    }
    int Day() const
    {
        return _iday;
    }
    int Hour() const
    {
        return _ihours;
    }
    int Mints() const
    {
        return _imins;
    }
    int Secs() const
    {
        return _isecs;
    }
    std::string LUnit() const
    {
        return _LUnit;
    }
    std::string TUnit() const
    {
        return _TUnit;
    }
    std::string MUnit() const
    {
        return _MUnit;
    }
    int NumofObsNodes() const
    {
        return _NObs;
    }
    int ObsNodeIndex(const int index) const
    {
        return _iobs[index];
    }
    double CalCulationTime() const
    {
        return _CalTm;
    }
    operator bool()
    {
        return _isValid;
    }
    bool ImportInputFiles();
    bool ExportInputFiles();
    bool ImportResultFiles();
    bool ExportResultFiles();
    bool DropResultFiles();
    bool DropCase();
    bool DropInputFiles();
    std::unique_ptr<std::string> GetImportResultFilesSQlStatement();
private:
    HydrusParameterFilesManager();
    void GetErrMessage(const std::string &filename);
    void OpenInputFiles();
    void OpenResultFiles();
    bool ParseSqlARRAY(const std::string &value, int *p, int nsize);
private:
    std::string _path;
    int _gid;
    QSqlQuery* _qry;
    bool _isInitial;
private:
    bool _isValid;
    bool _err;
    std::string _errMessage;
    std::string _Hed;
    int _NLayer;
    int _NS;
    int _HeadLine;
    int _iday;
    int _imonth;
    int _ihours;
    int _imins;
    int _isecs;
    int _NObs;
    double _CalTm;
    std::unique_ptr<int[]> _iobs;
    std::string _LUnit;
    std::string _TUnit;
    std::string _MUnit;
    std::string _status;
    std::unique_ptr<SelectorObject> _sel;
    std::unique_ptr<AtmosphObject> _atm;
    std::unique_ptr<ProfileObject> _pro;
    std::unique_ptr<ALevelObject> _alev;
    std::unique_ptr<TLevelObject> _tlev;
    std::unique_ptr<NodInfoObject> _nod;
    std::unique_ptr<ObsNodeObject> _obs;
    std::unique_ptr<BalanceObject> _bal;
    std::unique_ptr<SoluteObject> _sol[10];
};
#endif // HYDRUSPARAMETERFILESMANAGER_H
