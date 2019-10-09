
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

#ifndef EXPORTER_H
#define EXPORTER_H
#include <vector>
#include <string>
#include <memory>
class QSqlQuery;

class Exporter
{
public:
    static bool exportInputFiles(std::vector<int>& gids,const std::string& dest,std::shared_ptr<QSqlQuery> qry,bool bAscII=true);
    static bool exportInputFile(int gid,const std::string&dest,std::shared_ptr<QSqlQuery> qry,bool bAscII=true);
    static bool exportOutputFiles(std::vector<int>& gids,const std::string& dest,std::shared_ptr<QSqlQuery> qry,bool bAscII=true);
    static bool exportOutputFile(int gid,const std::string& dest,std::shared_ptr<QSqlQuery> qry,bool bAscII=true);
    static void SetLogFile(const std::string& logfile)
    {
        _logFile=logfile;
    }

private:
    static std::string _logFile;
    static void LogError(const std::string& err);
};

#endif // EXPORTER_H
