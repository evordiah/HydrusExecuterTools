
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

#ifndef OBSNODEOBJECT_H
#define OBSNODEOBJECT_H

#include <ostream>
#include <list>
#include <map>
#include <memory>
#include "IHydrusParameterFileObject.h"

namespace pqxx
{
    class connection;
}
class HydrusParameterFilesManager;
class ObsNodeObject:public IHydrusParameterFileObject
{
public:
    ObsNodeObject(const std::string& filename,HydrusParameterFilesManager *parent);
    ObsNodeObject(int gid, pqxx::connection &qry,HydrusParameterFilesManager *parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~ObsNodeObject();
    bool Save(const std::string& path);
    bool Save(std::ostream& out);
    std::string ToSqlStatement( int gid);
    bool open(const std::string& filename);
    bool open(int gid,pqxx::connection & qry);
private:
    void WriteHead(std::ostream& out);
    bool ParseLine(const char* p, int nsize, int lineindex);
    void SaveLine(std::ostream&out, int lineindex);
    std::string LineToSql( int gid, int lineindex);
private:
    bool _isValid;
    int _NObs;
    int _NS;
    int _tmCnt;
    std::unique_ptr<int[]> _iobs;
    std::unique_ptr<double[]> _time;
    std::unique_ptr<double[]> _head;
    std::unique_ptr<double[]> _flux;
    std::unique_ptr<double[]> _theta;
    std::unique_ptr<double[]> _conc;
    HydrusParameterFilesManager *_parent;
};


#endif // OBSNODEOBJECT_H
