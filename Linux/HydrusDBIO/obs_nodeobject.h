
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

#ifndef OBS_NODEOBJECT_H
#define OBS_NODEOBJECT_H
#include "obs_nodeparser.h"

class Obs_NodeObject : public Obs_NodeEncoder
{
public:
    Obs_NodeObject(std::istream& in);
    Obs_NodeObject(const std::string& filename);

    int NodeCount() const
    {
        return _nodes.size();
    }

    int Node(int index) const
    {
        return _nodes[index];
    }

    float Time(int index) const
    {
        return _data[index*_nFiledCnt];
    }

    float H(int index,int nodeindex) const
    {
        return _data[index*_nFiledCnt+nodeindex*3+1];
    }

    float Theta(int index,int nodeindex) const
    {
        return _data[index*_nFiledCnt+nodeindex*3+2];
    }

    float Flux(int index,int nodeindex) const
    {
        return _data[index*_nFiledCnt+nodeindex*3+3];
    }

    int LineSize() const
    {
        return _nLine;
    }

    virtual ~Obs_NodeObject(){}
    virtual void SaveAsObs_NodeFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const Obs_NodeObject& obj);
    friend std::istream& operator>>(std::istream& in, Obs_NodeObject& obj);
protected:
    Obs_NodeObject(){_nFiledCnt=0;}
    int _nFiledCnt;
};

#endif // OBS_NODEOBJECT_H
