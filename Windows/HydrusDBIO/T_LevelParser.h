
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

#ifndef T_LEVELPARSER_H
#define T_LEVELPARSER_H
#include <string>
#include <iostream>
#include <memory>

class T_LevelEncoder
{
public:
    T_LevelEncoder(const std::string& filename);

    virtual ~T_LevelEncoder();

    //return the whole size of file,including head
    unsigned int GetLength() const
    {
        unsigned int size=2*sizeof(int)+_nLine*(sizeof(float)*22);
        size+=(3*sizeof(int)+_DateandTime.size()+_TUnit.size()+_LUnit.size());
        return size;
    }
    friend std::ostream& operator<<(std::ostream& out,const T_LevelEncoder& T_Level);
    bool ParseFile(const std::string& filename);
protected:
    T_LevelEncoder()
    {
        _nLine=0;
        _FileLength=0;
    }
    bool ParseHead(std::istream &in);
    bool ParseFile(std::istream& in);
    std::unique_ptr<float[]> _data;
    const int _LineLength=200;
    int _nLine;
    int _FileLength;
    std::string _DateandTime;
    std::string _TUnit;
    std::string _LUnit;
};

#endif // T_LEVELPARSER_H
