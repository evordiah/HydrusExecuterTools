
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

#ifndef IMPORTHYDRUSINPUTFILE_H
#define IMPORTHYDRUSINPUTFILE_H
#include <QString>
#include <memory>

class AtmosphObject;
class ProfileObject;
class SelectorObject;
class QSqlQuery;

class ImportHydrusinputFile
{
public:
    ImportHydrusinputFile();
    ~ImportHydrusinputFile();
    bool Execute(std::shared_ptr<QSqlQuery> qry);
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
    void Filename(const std::string &value);
private :
    int _gid;
    bool _valid;
    std::unique_ptr<AtmosphObject> _paobj;
    std::unique_ptr<ProfileObject> _ppobj;
    std::unique_ptr<SelectorObject> _psobj;
    QString GetAtmosphsql();
    QString GetProfilesql();
    QString GetSelectorsql();
};

#endif // IMPORTHYDRUSINPUTFILE_H
