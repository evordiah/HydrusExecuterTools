
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

#ifndef PROFILEOBJECT_H
#define PROFILEOBJECT_H

#include <string>
#include <vector>
#include <istream>
#include <memory>
#include "IHydrusParameterFileObject.h"

namespace pqxx
{
    class connection;
}
class SelectorObject;

class ProfileObject:public IHydrusParameterFileObject
{
public:
    ProfileObject(const std::string& filename,SelectorObject* parent);
    ProfileObject(int gid, pqxx::connection &qry,SelectorObject* parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~ProfileObject();
    bool Save(const std::string& path);
    bool Save(std::ostream& out);
    std::string ToSqlStatement( int gid);
    bool open(const std::string& filename);
    bool open(int gid,pqxx::connection& qry);
private:
    bool _isValid;
    SelectorObject* _sel;
    int _NumNP;
    int _NS;
    int _iTemp;
    int _iEquil;
    int _NObs;
    std::unique_ptr<int[]> _n;
    std::unique_ptr<double[]> _x;
    std::unique_ptr<double[]> _hNew;
    std::unique_ptr<int[]> _MatNum;
    std::unique_ptr<int[]> _LayNum;
    std::unique_ptr<double[]> _Beta;
    std::unique_ptr<double[]> _Ah;
    std::unique_ptr<double[]> _Ak;
    std::unique_ptr<double[]> _Ath;
    std::unique_ptr<double[]> _Conc;
    std::unique_ptr<double[]> _Sorb;
    std::unique_ptr<int[]> _iObs;
private:
    bool ParseLine(const std::string& line,const std::string& lineformat, const std::vector<void *> &values);
    bool ParseLine(const std::string& line,  int lineindex);
    bool ParseLine(const std::string& line,  int lineindex, int NS);
    void Initial();
    char boolalpha(bool value)
    {
        return value?'t':'f';
    }
    void SaveLine(std::ostream& out,  int lineindex);
    void SaveLine(std::ostream& out,  int lineindex,  int NS);
};

#endif // PROFILEOBJECT_H
