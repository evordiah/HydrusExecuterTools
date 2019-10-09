
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

#ifndef NOD_INFPARSER_H
#define NOD_INFPARSER_H
#include <string>
#include <iostream>
#include <vector>
#include <memory>

class Nod_InfEncoder
{
    public:
        Nod_InfEncoder(const std::string& filename);
        virtual ~Nod_InfEncoder();
        //return the whole size of file,including head
        unsigned int GetLength() const
        {
            unsigned int size=3*sizeof(int)+_DateandTime.size()+_TUnit.size()+_LUnit.size();
            size+=3*sizeof(int);
            size+=vec_time.size()*sizeof(float);
            size+=_nLine*(sizeof(float)*11);
            return size;
        }
        friend std::ostream& operator<<(std::ostream& out,const Nod_InfEncoder& Nod_info);
        bool ParseFile(const std::string& filename);
    protected:
        Nod_InfEncoder()
        {
            _nLine=0;
            _FileLength=0;
        }
        void ParseFileHead(std::istream& in);
        bool GetTime(std::istream& in);
        void ParsePartData(std::istream& in, float **pos, int &nLine);

        bool ParseFile(std::istream& in);

        std::unique_ptr<float[]> _data;
        std::vector<float> vec_time;
        std::string _DateandTime;
        std::string _TUnit;
        std::string _LUnit;
        const int _LineLength=70;
        int _nLine;
        int _FileLength;
};

#endif // NOD_INFPARSER_H
