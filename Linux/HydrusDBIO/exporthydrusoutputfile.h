
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

#ifndef EXPORTHYDRUSOUTPUTFILE_H
#define EXPORTHYDRUSOUTPUTFILE_H
#include <string>
#include <memory>

class A_LevelObject;
class T_LevelObject;
class NodeInfoObject;
class BalanceObject;
class Obs_NodeObject;
class QSqlQuery;

class ExportHydrusOutputFile
{
public:
    ExportHydrusOutputFile(std::shared_ptr<QSqlQuery> qry);
    ~ExportHydrusOutputFile();
    bool Execute(const std::string& value);
	void Gid(int value);
    bool operator!()
    {
        return !_valid;
    }
    operator bool()
    {
        return _valid;
    }
private :
    int _gid;
    bool _valid;
    std::shared_ptr<QSqlQuery>  _pqry;
    std::unique_ptr<A_LevelObject>  GetALevelSql();
    std::unique_ptr<T_LevelObject> GetTLevelSql();
    std::unique_ptr<NodeInfoObject> GetNodinfoSql();
    std::unique_ptr<BalanceObject> GetBalancesql();
    std::unique_ptr<Obs_NodeObject> GetObsNodesql();
};

#endif // EXPORTHYDRUSOUTPUTFILE_H
