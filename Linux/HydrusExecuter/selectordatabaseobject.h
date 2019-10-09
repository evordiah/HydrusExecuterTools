
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

#ifndef SELECTORDATABASEOBJECT_H
#define SELECTORDATABASEOBJECT_H
#include "selectorobject.h"
class QSqlQuery;
class SelectorDataBaseObject : public SelectorObject
{
public:
    SelectorDataBaseObject(int gid,QSqlQuery& qry);
private:
    void FillMat(const std::string& value);
    void FillPrintTime(const std::string& value);
    void FillRootGrowth(const std::string& day, const std::string& length, int rootcnt);
    void FillPoptm(const std::string& value);
};

#endif // SELECTORDATABASEOBJECT_H
