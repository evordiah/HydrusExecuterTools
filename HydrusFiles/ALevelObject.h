
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

#ifndef ALEVELOBJECT_H
#define ALEVELOBJECT_H

#include <ostream>
#include <list>
#include <memory>
#include "IHydrusParameterFileObject.h"

namespace  fwzformat
{
    class ffmt_proxy;
    template<typename Rhs>
    std::ostream & operator<<(ffmt_proxy const& q,Rhs const& rhs);
}
class QSqlQuery;
class HydrusParameterFilesManager;
class ALevelObject:public IHydrusParameterFileObject
{
    struct ALevelRecord
    {
        ALevelRecord(const char* pline);
        ALevelRecord(QSqlQuery &qry);
        double Time;
        double sum_rTop;
        double sum_rRoot;
        double sum_vTop;
        double sum_vRoot;
        double sum_vBot;
        double hTop;
        double hRoot;
        double hBot;
        int ALevel;
    } ;
    friend std::ostream& operator<<(std::ostream& os,const ALevelRecord& arec);
    template<typename Rhs>
    friend std::ostream & fwzformat::operator<<(fwzformat::ffmt_proxy const& q,Rhs const& rhs);
public:
    ALevelObject(const std::string& filename,HydrusParameterFilesManager *parent);
    ALevelObject(int gid, QSqlQuery &qry,HydrusParameterFilesManager *parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~ALevelObject();
    bool Save(const std::string& path);
    std::string ToSqlStatement(const int gid);
    bool open(const std::string& filename);
    bool open(int gid,QSqlQuery& qry);
private:
    bool _isValid;
    std::list<std::unique_ptr<ALevelRecord>> _Recs;
    HydrusParameterFilesManager *_parent;
};
std::ostream& operator<<(std::ostream& os,const ALevelObject::ALevelRecord& arec);
template<>
std::ostream & fwzformat::operator<<(fwzformat::ffmt_proxy const& q,ALevelObject::ALevelRecord const& rhs);

#endif // ALEVELOBJECT_H
