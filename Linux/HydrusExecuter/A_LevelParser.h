
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

#ifndef A_LEVELPARSER_H
#define A_LEVELPARSER_H
#include <string>
#include <iostream>
#include <memory>

class A_LevelEncoder
{
    public:
        A_LevelEncoder(const std::string& filename);
        virtual ~A_LevelEncoder();
        //return the whole size of file,including head
        unsigned int GetLength() const
        {
            return 2*sizeof(int)+_nLine*(sizeof(float)*10);
        }
        bool ParseFile(const std::string& filename);
        friend std::ostream& operator<<(std::ostream& out,const A_LevelEncoder& A_Level);
    protected:
        A_LevelEncoder()
        {
            _nLine=0;
            _FileLength=0;
        }
        bool ParseFile(std::istream& in);
        std::unique_ptr<float[]> _data;
        const int _LineLength=80;
        int _nLine;
        int _FileLength;
};
#endif // A_LEVELPARSER_H
