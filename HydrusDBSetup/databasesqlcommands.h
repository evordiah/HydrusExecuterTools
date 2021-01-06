
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

#ifndef DATABASESQLCOMMANDS_H
#define DATABASESQLCOMMANDS_H
#include <vector>
#include <string>

class DataBaseSQLCommands
{
public:
    DataBaseSQLCommands();
    DataBaseSQLCommands( int tablecount, int gidcount);
    std::string GetCreateDbSqlCommand(const std::string &dbname);
    std::string GetCreateTablesSqlCommand();
protected:
    std::string CreateALevel();
    std::string CreateTLevel();
    std::string CreateNodeinfo();
    std::string CreateObsNode();
    std::string CreateSolute();
    std::string CreateProfile();
    std::string CreateAtmosph();
    std::string CreateSelector();
    std::string CreateFunction();
private:
    bool _bPartitionTable;
    int _tablecnt1; //for table such as a_level,t_level,atmosph,solute
    int _idcnt1;    //for table such as a_level,t_level,atmosph,solute
    int _tablecnt2; //for table such as nod_info,profile,obsnode
    const int _idcnt2; //for table such as nod_info,profile,obsnode
    const int _maxid;
};

#endif // DATABASESQLCOMMANDS_H
