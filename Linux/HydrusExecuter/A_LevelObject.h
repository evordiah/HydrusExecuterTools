
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

#ifndef A_LEVELOBJECT_H
#define A_LEVELOBJECT_H
#include <iostream>
#include <memory>
#include "A_LevelParser.h"

class A_LevelObject:public A_LevelEncoder
{
public:
    A_LevelObject(std::istream& in);
    A_LevelObject(const std::string& filename);

    float Time(int index) const
    {
        return _data[index*10];
    }

    float sum_rTop(int index) const
    {
        return _data[index*10+1];
    }

    float sum_rRoot(int index) const
    {
        return _data[index*10+2];
    }

    float sum_vTop(int index) const
    {
        return _data[index*10+3];
    }

    float sum_vRoot(int index) const
    {
        return _data[index*10+4];
    }

    float sum_vBot(int index) const
    {
        return _data[index*10+5];
    }

    float hTop(int index) const
    {
        return _data[index*10+6];
    }

    float hRoot(int index) const
    {
        return _data[index*10+7];
    }

    float hBot(int index) const
    {
        return _data[index*10+8];
    }

    int A_level(int index) const
    {
        return (int)_data[index*10+9];
    }

    int LineSize() const
    {
        return _nLine;
    }

    virtual ~A_LevelObject();
    virtual void SaveAsA_LevelFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const A_LevelObject& obj);
    friend std::istream& operator>>(std::istream& in, A_LevelObject& obj);
protected:
    A_LevelObject(){}
    std::string FormatFloat(float value);
    void FormatLine(std::ostream &out, const float *pLine);
};

#endif // A_LEVELOBJECT_H
