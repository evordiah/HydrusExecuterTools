
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

#ifndef EXPORTHYDRUSINPUTFILE_H
#define EXPORTHYDRUSINPUTFILE_H
#include <QString>
#include <memory>

class AtmosphObject;
class ProfileObject;
class SelectorObject;
class QSqlQuery;

class ExportHydrusInputFile
{
public:
    ExportHydrusInputFile(std::shared_ptr<QSqlQuery> qry);
    ~ExportHydrusInputFile();
    bool Execute(const std::string& value);
    void Gid(int value)
    {
        _gid=value;
    }
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
    std::unique_ptr<AtmosphObject>  GetAtmosphsql();
    std::unique_ptr<ProfileObject> GetProfilesql();
    std::unique_ptr<SelectorObject> GetSelectorsql();
};

#endif // EXPORTHYDRUSINPUTFILE_H
