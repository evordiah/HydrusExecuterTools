
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

#ifndef NODINFOWITHOUTSOLUTEOBJECT_H
#define NODINFOWITHOUTSOLUTEOBJECT_H

#include <ostream>
#include <list>
#include <map>
#include <memory>
#include "IHydrusParameterFileObject.h"

class QSqlQuery;
class HydrusParameterFilesManager;
class NodInfoObject:public IHydrusParameterFileObject
{
    struct NodinfoRecord
    {
        NodinfoRecord(const char* pline,const int NS);
        NodinfoRecord(QSqlQuery &qry,const int NS);
        int Node;
        double Depth;
        double Head;
        double Moisture;
        double K;
        double C;
        double Flux;
        double Sink;
        int Kappa;
        double vdivKsTop;
        double Temp;
        std::unique_ptr<double[]> _Conc;
        std::unique_ptr<double[]> _Sorb;
    } ;
public:
    NodInfoObject(const std::string& filename, HydrusParameterFilesManager *parent);
    NodInfoObject(int gid, QSqlQuery &qry,HydrusParameterFilesManager *parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~NodInfoObject();
    bool Save(const std::string& path);
    std::string ToSqlStatement(const int gid);
    bool open(const std::string& filename);
    bool open(int gid,QSqlQuery& qry);
private:
    void WriteHead(std::ostream& out);
    void WriteSection(std::ostream &out, double time,std::list<std::unique_ptr<NodinfoRecord>>& lst,const int NS);
    void SaveLine(std::ostream&out,const NodinfoRecord& nrect,const int NS);
    std::string ToSqlStatement(const NodinfoRecord& nrect,const int NS);
private:
    bool _isValid;
    std::map<double,std::list<std::unique_ptr<NodinfoRecord>>> _Recs;
    HydrusParameterFilesManager *_parent;
};
#endif // NODINFOWITHOUTSOLUTEOBJECT_H
