
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

#ifndef IHYDRUSPARAMETERFILEOBJECT_H
#define IHYDRUSPARAMETERFILEOBJECT_H

#include <string>

namespace pqxx
{
    class connection;
}
class IHydrusParameterFileObject
{
public:
    virtual operator bool()=0;
    virtual bool Save(const std::string& path)=0;
    virtual std::string ToSqlStatement( int gid)=0;
    virtual bool open(const std::string& filename)=0;
    virtual bool open(int gid, pqxx::connection &qry)=0;
};

#endif // IHYDRUSPARAMETERFILEOBJECT_H
