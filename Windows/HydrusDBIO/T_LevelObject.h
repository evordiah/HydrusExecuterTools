
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

#ifndef T_LEVELOBJECT_H
#define T_LEVELOBJECT_H
#include <iostream>
#include <memory>
#include "T_LevelParser.h"

class T_LevelObject:public T_LevelEncoder
{
public:
    T_LevelObject(std::istream& in);
    T_LevelObject(const std::string& filename);

    const float* operator[](int index) const
    {
        return _data.get()+index*22;
    }

    float Time(int index) const
    {
        return _data[index*22];
    }

    float rTop(int index) const
    {
        return _data[index*22+1];
    }

    float rRoot(int index) const
    {
        return _data[index*22+2];
    }

    float vTop(int index) const
    {
        return _data[index*22+3];
    }

    float vRoot(int index) const
    {
        return _data[index*22+4];
    }

    float vBot(int index) const
    {
        return _data[index*22+5];
    }

    float sum_rTop(int index) const
    {
        return _data[index*22+6];
    }

    float sum_rRoot(int index) const
    {
        return _data[index*22+7];
    }

    float sum_vTop(int index) const
    {
        return _data[index*22+8];
    }

    float sum_vRoot(int index) const
    {
        return _data[index*22+9];
    }

    float sum_vBot(int index) const
    {
        return _data[index*22+10];
    }

    float hTop(int index) const
    {
        return _data[index*22+11];
    }

    float hRoot(int index) const
    {
        return _data[index*22+12];
    }

    float hBot(int index) const
    {
        return _data[index*22+13];
    }

    float RunOff(int index) const
    {
        return _data[index*22+14];
    }

    float sum_RunOff(int index) const
    {
        return _data[index*22+15];
    }

    float Volume(int index) const
    {
        return _data[index*22+16];
    }

    float sum_Infil(int index) const
    {
        return _data[index*22+17];
    }

    float sum_Evap(int index) const
    {
        return _data[index*22+18];
    }

    int TLevel(int index) const
    {
        return (int)_data[index*22+19];
    }

    float Cum_WTrans(int index) const
    {
        return _data[index*22+20];
    }

    float SnowLayer(int index) const
    {
        return _data[index*22+21];
    }

    int LineSize() const
    {
        return _nLine;
    }

    virtual ~T_LevelObject()
    {
    }

    virtual void SaveAsT_LevelFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const T_LevelObject& obj);
    friend std::istream& operator>>(std::istream& in, T_LevelObject& obj);
protected:
    T_LevelObject(){}
    std::string FormatFloat(float value);
    void FormatLine(std::ostream &out, const float *pLine);
};

#endif // T_LEVELOBJECT_H
