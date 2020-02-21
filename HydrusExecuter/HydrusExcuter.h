
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

#ifndef HYDRUSEXCUTER_H
#define HYDRUSEXCUTER_H
#include <string>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <queue>
#include <QDir>
#include <memory>
#include "HydrusParameterFilesManager.h"

class HydrusExcuter
{
public:
    HydrusExcuter(const std::string& exepath);
    virtual ~HydrusExcuter();
    void DataBase(const QSqlDatabase& db);
    void operator()();
    virtual void Execute();
protected:
    bool InitialThread();
    bool PreapareParameterFile(int gid);
    bool ExecuteHydrus(int gid);
    void ClearTempFile();
    virtual bool GetId(int & gid);
protected:
    bool _valid;
    QSqlDatabase _db;
    std::unique_ptr<QSqlQuery> _pqry;
    std::unique_ptr<HydrusParameterFilesManager> _HydrusFilesManager;
	QDir _exepath;
	QDir _currentprocesspath;
};

#endif // HYDRUSEXCUTER_H
