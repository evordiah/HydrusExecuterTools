
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

#ifndef SOLUTEOBJECT_H
#define SOLUTEOBJECT_H

#include <ostream>
#include <list>
#include <memory>
#include "IHydrusParameterFileObject.h"

namespace pqxx
{
    class connection;
    class row;
}
class HydrusParameterFilesManager;

class SoluteObject:public IHydrusParameterFileObject
{
    struct SoluteRecord
    {
        SoluteRecord(const char* pline, int NObs);
        SoluteRecord(pqxx::row &row, int NOBS);
        double Time;
        double cvTop;
        double cvBot;
        double sum_cvTop;
        double sum_cvBot;
        double sum_cvCh0;
        double sum_cvCh1;
        double cTop;
        double cRoot;
        double cBot;
        double cvRoot;
        double sum_cvRoot;
        double sum_cvNEql;
        int TLevel;
        double cGWL;
        double cRunOff;
        double sum_cRunOff;
        double cv[3];
        double sumcv[3];
    } ;
public:
    SoluteObject(const std::string& filename,HydrusParameterFilesManager* parent);
    SoluteObject(int gid, pqxx::connection &qry,HydrusParameterFilesManager* parent, int index);
    operator bool()
    {
        return _isValid;
    }
    virtual ~SoluteObject();
    bool Save(const std::string& path);
    bool Save(std::ostream& out);
    std::string ToSqlStatement( int gid);
    bool open(const std::string& filename);
    bool open(int gid,pqxx::connection& qry);
private:
    bool _isValid;
    int _FileIndex;
    int _NObs;
    std::list<std::unique_ptr<SoluteRecord>> _Recs;
    HydrusParameterFilesManager* _parent;
private:
    void SaveLine(std::ostream& os,const SoluteObject::SoluteRecord& srec);
    std::string ToSqlStatement(const SoluteRecord &srec);
};
#endif // SOLUTEOBJECT_H
