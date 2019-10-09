
/****************************************************************************** 
 * 
 * 
 *  Copyright (c) 2019, Wenzhao Feng.
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
#include <memory>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <QDir>

class HydrusExcuter
{
public:
    static std::queue<std::string> bufresultsqueue;
    static std::mutex bufResMutex;
    static std::condition_variable bufCondVar;
    static std::queue<int> gids;
    static std::mutex gidsMutex;
public:
    HydrusExcuter(const std::string& exepath);
    virtual ~HydrusExcuter();
    void DataBase(const QSqlDatabase& db);
    void operator()();
    virtual void Execute();
protected:
    bool InitialProcess();
    bool PreapareParameterFile(int gid);
    void ExecuteHydrus(int gid);
    void CompressResults(int gid);
    void ClearTempFile();
    void LogError(int gid, const std::string &error);
    virtual bool GetId(int & gid);
protected:
    bool _valid;
    QSqlDatabase _db;
    std::shared_ptr<QSqlQuery> _pqry;
	QDir _exepath;
	QDir _currentprocesspath;
};

#endif // HYDRUSEXCUTER_H
