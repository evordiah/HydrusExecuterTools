
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

#ifndef NODEINFOOBJECT_H
#define NODEINFOOBJECT_H
#include "Nod_InfParser.h"
#include <istream>
#include <vector>
#include <memory>

class NodeInfoObject:public Nod_InfEncoder
{
public:
    NodeInfoObject(std::istream& in);
    NodeInfoObject(const std::string& filename);
    virtual ~NodeInfoObject()
    {
    }

    float Time(int index) const
    {
        return vec_time[index];
    }

    unsigned int TimeNum() const
    {
        return vec_time.size();
    }

    int NodeNum() const
    {
        return _NodeNum;
    }

    float Depth(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+1];
    }

    float Head(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+2];
    }

    float Moisture(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+3];
    }

    float K(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+4];
    }

    float C(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+5];
    }

    float Flux(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+6];
    }

    float Sink(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+7];
    }

    float Kappa(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+8];
    }

    float vdivKsTop(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+9];
    }

    float Temp(int timeindex,int nodeindex)
    {
        return _data[(timeindex*_NodeNum+nodeindex)*11+10];
    }

    virtual void SaveAsNod_InfFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const NodeInfoObject& obj);
    friend std::istream& operator>>(std::istream& in, NodeInfoObject& obj);
protected:
    NodeInfoObject()
    {
        _NodeNum=0;
    }
    std::string GetFileHead();
    std::string GetSectionHead(int index);
    void FormatSection(int sectioninx, int recnum, std::ostream &out);
    std::string FormatFloat(float value, unsigned int precision, int width);
    void FormatLine(std::ostream &out, const float *pLine);
private:
    int _NodeNum;
};

#endif // NODEINFOOBJECT_H
