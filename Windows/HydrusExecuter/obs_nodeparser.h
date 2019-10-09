
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

#ifndef OBS_NODEPARSERH_H
#define OBS_NODEPARSERH_H
#include <string>
#include <iostream>
#include <memory>
#include <vector>

class Obs_NodeEncoder
{
public:
    Obs_NodeEncoder(const std::string& filename);
    virtual ~Obs_NodeEncoder();
    //return the whole size of file,including head
    unsigned int GetLength() const
    {
        int nFldCnt=_nodes.size()*3+1;
        unsigned int size=(3+_nodes.size())*sizeof(int)+_nLine*(sizeof(float)*nFldCnt);
        size+=(3*sizeof(int)+_DateandTime.size()+_TUnit.size()+_LUnit.size());
        return size;
    }
    bool ParseFile(const std::string& filename);
    friend std::ostream& operator<<(std::ostream& out,const Obs_NodeEncoder& obsencoder);
protected:
    Obs_NodeEncoder()
    {
        _nLine=0;
        _FileLength=0;
    }
    bool ParseFile(std::istream& in);
    bool ParseHead(std::istream& in);
    std::vector<int> _nodes;
    std::unique_ptr<float[]> _data;
    const int _LineLength=280;
    int _nLine;
    int _FileLength;
    std::string _DateandTime;
    std::string _TUnit;
    std::string _LUnit;
};

#endif // OBS_NODEPARSERH_H
