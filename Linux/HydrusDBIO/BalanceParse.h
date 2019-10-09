
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

#ifndef BALANCEPARSE_H
#define BALANCEPARSE_H
#include <string>
#include <vector>

class BalanceEncoder
{
public:
    BalanceEncoder(const std::string& filename);
    virtual ~BalanceEncoder();
    bool ParseFile(const std::string& filename);
    unsigned int GetLength() const
    {
        unsigned int size=2+_timenum*sizeof(float)+vec_num.size()*sizeof(short)+vec_data.size()*sizeof(float);
        size+=(3*sizeof(int)+_DateandTime.size()+_TUnit.size()+_LUnit.size());
        size+=sizeof(double);
        return size;
    }
    friend std::ostream& operator<<(std::ostream& out,const BalanceEncoder& Balance);
protected:
    BalanceEncoder()
    {
        _timenum=0;
        _regionnum=0;
    }
    bool ParseFile(std::istream& in);

    void ParseFileHead(std::istream& in);
    bool GetTime(std::istream& in);
    bool ParseSubregionNum(std::istream& in);
    bool ParsePartData(std::istream& in,int num);

    char _timenum;
    char _regionnum;
    std::vector<float> vec_time;
    std::vector<short> vec_num;
    std::vector<float> vec_data;
    std::string _DateandTime;
    std::string _TUnit;
    std::string _LUnit;
    double _caltime;
};

#endif // BALANCEPARSE_H
