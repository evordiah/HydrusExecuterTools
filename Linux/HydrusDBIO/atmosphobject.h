
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

#ifndef ATMOSPHOBJECT_H
#define ATMOSPHOBJECT_H
#include "AtmosphParser.h"
#include <iostream>
#include <memory>

class AtmosphObject:public AtmosphEncoder
{
public:
    AtmosphObject();
    AtmosphObject(int maxAL, float hCritS=1000);
    AtmosphObject(std::istream& in);
    AtmosphObject(const std::string& filename);
    virtual ~AtmosphObject();

    virtual void MaxAL(int maxAL)
    {
        if(maxAL<=0 ||_maxAL==maxAL )
        {
            return;
        }
        _maxAL=maxAL;
        _data.reset(new float[maxAL*5]);
    }
    virtual int MaxAL()
    {
        return _maxAL;
    }

    void HCritS(float val)
    {
        _hCrits=val;
    }
    float HCritS()
    {
        return _hCrits;
    }

    virtual void TAtm(int ind,float val)
    {
        _data[ind*5]=val;
    }
    virtual float TAtm(int ind)
    {
        return _data[ind*5];
    }

    virtual void Prec(int ind,float val)
    {
        _data[ind*5+1]=val;
    }
    virtual float Prec(int ind)
    {
        return _data[ind*5+1];
    }

    virtual void Evap(int ind,float val)
    {
        _data[ind*5+2]=val;
    }
    virtual float Evap(int ind)
    {
        return  _data[ind*5+2];
    }

    virtual void Trasp(int ind,float val)
    {
        _data[ind*5+3]=val;
    }
    virtual float Trasp(int ind)
    {
        return _data[ind*5+3];
    }

    virtual void HCritA(int ind,float val)
    {
        _data[ind*5+4]=val;;
    }
    virtual float HCritA(int ind)
    {
        return _data[ind*5+4];
    }

    virtual void SaveAsAtmosphFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const AtmosphObject& obj);
    friend std::istream& operator>>(std::istream& in, AtmosphObject& obj);
protected:
    std::string GetHead();
    virtual std::string GetBlockI();
    std::string GetEnd();
};

#endif // ATMOSPHOBJECT_H
