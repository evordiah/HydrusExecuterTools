
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

#ifndef PROFILEOBJECT_H
#define PROFILEOBJECT_H
#include <string>
#include <iostream>
#include <memory>
#include "ProfileParser.h"

class ProfileObject:public ProfileEncoder
{
public:
    ProfileObject();
    ProfileObject(float depth,int nodecnt,float interval=0,int observercnt=0,float maxrootdepth=200);
    ProfileObject(const std::string& filename);
    ProfileObject(std::istream& in);
    virtual ~ProfileObject();

    void Depth(float dp)
    {
        _depth=dp;
    }
    float Depth()
    {
        return _depth;
    }

    void NodeCount(int val)
    {
        if(val <=0 || val==_nodecnt)
        {
            return;
        }
        _nodecnt=val;
        _interval=_depth/(val-1);
        AllocateMemory();
    }
    int NodeCount()
    {
        return _nodecnt;
    }

    void Interval(float val)
    {
        if(val <=0 || val==_interval)
        {
            return;
        }
        _interval=val;
        _nodecnt=_depth/_interval+1;
        AllocateMemory();
    }

    void ObserversationCount(int val)
    {
        if(val==_observercnt)
        {
            return;
        }
        if(val<0)
        {
            val=0;
        }
        _observercnt=val;

        if(_observercnt)
        {
            _observeNodeid.reset(new int[val]);
        }
        else
        {
            _observeNodeid=nullptr;
        }
    }
    int ObserversationCount()
    {
        return _observercnt;
    }

    void Coord(int ind,float val)
    {
        _xcoord[ind]=val;
    }
    float Coord(int ind)
    {
        return _xcoord[ind];
    }

    void H(int ind,float val)
    {
        _h[ind]=val;
    }
    float H(int ind)
    {
        return _h[ind];
    }
    void H(int from,int to,float val)
    {
        for(int i=from; i<=to; ++i)
        {
            _h[i]=val;
        }
    }

    void Mat(int ind,int val)
    {
        _mat[ind]=val;
    }
    int Mat(int ind)
    {
        return _mat[ind];
    }
    void Mat(int from,int to,int val)
    {
        for(int i=from; i<=to; ++i)
        {
            _mat[i]=val;
        }
    }

    void Lay(int ind,int val)
    {
        _lay[ind]=val;
    }
    int Lay(int ind)
    {
        return _lay[ind];
    }
    void Lay(int from,int to,int val)
    {
        for(int i=from; i<=to; ++i)
        {
            _lay[i]=val;
        }
    }

    void Beta(int ind,float val)
    {
        _beta[ind]=val;
    }
    float Beta(int ind)
    {
        return _beta[ind];
    }
    void Beta(int from,int to,float val)
    {
        for(int i=from; i<=to; ++i)
        {
            _beta[i]=val;
        }
    }

    void Ah(int ind,float val)
    {
        _Ah[ind]=val;
    }
    float Ah(int ind)
    {
        return _Ah[ind];
    }
    void Ah(int from,int to,float val)
    {
        for(int i=from; i<=to; ++i)
        {
            _Ah[i]=val;
        }
    }

    void Ak(int ind,float val)
    {
        _Ak[ind]=val;
    }
    float Ak(int ind)
    {
        return _Ak[ind];
    }
    void Ak(int from,int to,float val)
    {
        for(int i=from; i<=to; ++i)
        {
            _Ak[i]=val;
        }
    }

    void Ath(int ind,float val)
    {
        _Ath[ind]=val;
    }
    float Ath(int ind)
    {
        return _Ath[ind];
    }
    void Ath(int from,int to,float val)
    {
        for(int i=from; i<=to; ++i)
        {
            _Ath[i]=val;
        }
    }

    void ObservationID(int ind,int nodeid)
    {
        _observeNodeid[ind]=nodeid;
    }
    std::string GetObservationIDs();

    void SaveAsProfileFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const ProfileObject& obj);
    friend std::istream& operator>>(std::istream& in, ProfileObject& obj);

protected:
    std::string Format(float val);
    std::string GetHead();
    std::string GetMiddleBlock();
    std::string GetEnd();
    void GenerateCoords();
    //unit is cm
    void GenerateBeta(float maxroot, float interval);
    float _interval;
};

#endif // PROFILEOBJECT_H
