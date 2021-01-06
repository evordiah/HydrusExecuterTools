
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

#ifndef ATMOSOBJECT_H
#define ATMOSOBJECT_H

#include <string>
#include <vector>
#include <ostream>
#include <memory>
#include "IHydrusParameterFileObject.h"

namespace pqxx
{
    class connection;
}
class SelectorObject;

class AtmosphObject:public IHydrusParameterFileObject
{
public:
    AtmosphObject(const std::string& filename,SelectorObject* sel);
    AtmosphObject(int gid, pqxx::connection &qry,SelectorObject* sel);
    operator bool()
    {
        return _isValid;
    }
    virtual ~AtmosphObject();
    bool Save(const std::string& path);
    bool Save(std::ostream& out);
    std::string ToSqlStatement( int gid);
    bool open(const std::string& filename);
    bool open(int gid,pqxx::connection &qry);
private:
    bool _isValid;
    SelectorObject* _sel;
    int _MaxAl;
    bool lDailyVar,lSinusVar,lLAI,lBCCycles,lIntercep,lDummy;
    double _hCritS;
    std::unique_ptr<double[]> _tAtm;
    std::unique_ptr<double[]> _Prec;
    std::unique_ptr<double[]> _rSoil;
    std::unique_ptr<double[]> _rRoot;
    std::unique_ptr<double[]> _hCritA;
    std::unique_ptr<double[]> _hT;
    std::unique_ptr<double[]> _tTop;
    std::unique_ptr<double[]> _tBot;
    std::unique_ptr<double[]> _Ampl;
    std::unique_ptr<double[]> _cTop;
    std::unique_ptr<double[]> _cBot;
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

#endif // ATMOSOBJECT_H
