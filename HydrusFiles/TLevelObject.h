
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

#ifndef TLEVELOBJECT_H
#define TLEVELOBJECT_H

#include <ostream>
#include <list>
#include <memory>
#include "IHydrusParameterFileObject.h"


namespace  fwzformat
{
    struct ffmt_proxy;
    template<typename Rhs>
    std::ostream & operator<<(ffmt_proxy const& q,Rhs const& rhs);
}

namespace pqxx
{
    class connection;
    class row;
}
class HydrusParameterFilesManager;

class TLevelObject:public IHydrusParameterFileObject
{
public:
	struct TLevelRecord
    {
        TLevelRecord(const char* pline);
        TLevelRecord(pqxx::row &row);
        double Time;
        double rTop;
        double rRoot;
        double vTop;
        double vRoot;
        double vBot;
        double sum_rTop;
        double sum_rRoot;
        double sum_vTop;
        double sum_vRoot;
        double sum_vBot;
        double hTop;
        double hRoot;
        double hBot;
        double RunOff;
        double sum_RunOff;
        double Volume;
        double sum_Infil;
        double sum_Evap;
        int TLevel;
        double Sum_WTrans;
        double SnowLayer;
    } ;
    friend std::ostream& operator<<(std::ostream& os,const TLevelRecord& trec);
    template<typename Rhs>
    friend std::ostream & fwzformat::operator<<(fwzformat::ffmt_proxy const& q,Rhs const& rhs);
public:
    TLevelObject(const std::string& filename,HydrusParameterFilesManager* parent);
    TLevelObject(int gid, pqxx::connection &qry,HydrusParameterFilesManager* parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~TLevelObject();
    bool Save(const std::string& path);
    bool Save(std::ostream& out);
    std::string ToSqlStatement( int gid);
    bool open(const std::string& filename);
    bool open(int gid,pqxx::connection& qry);
private:
    bool _isValid;
    std::string _Hed;
    int _HeadLine;
    int _iday;
    int _imonth;
    int _ihours;
    int _imins;
    int _isecs;
    std::list<std::unique_ptr<TLevelRecord>> _Recs;
    HydrusParameterFilesManager* _parent;
    bool ParseHead(std::istream& in);
};
std::ostream& operator<<(std::ostream& os,const TLevelObject::TLevelRecord& trec);
template<>
std::ostream & fwzformat::operator<<(fwzformat::ffmt_proxy const& q,TLevelObject::TLevelRecord const& rhs);
#endif // TLEVELOBJECT_H
