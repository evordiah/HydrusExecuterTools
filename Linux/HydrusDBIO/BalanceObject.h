
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

#ifndef BALANCEOBJECT_H
#define BALANCEOBJECT_H
#include "BalanceParse.h"
#include <vector>
#include <istream>
#include <memory>

class BalanceObject:public BalanceEncoder
{
public:
    BalanceObject(std::istream& in);
    BalanceObject(const std::string& filename);
    virtual ~BalanceObject()
    {
    }

    int SugRegionNum() const
    {
        return _regionnum;
    }

    float Time(int index) const
    {
        return vec_time[index];
    }

    unsigned int TimeNum() const
    {
        return vec_time.size();
    }

    float Length(int timeindex,int subregionindex) const
    {
        int index=timeindex==0?0:(_headnum+(timeindex-1)*_othernum);
        return vec_data[index+subregionindex];
    }

    float W_volume(int timeindex,int subregionindex) const
    {
        int index=timeindex==0?0:(_headnum+(timeindex-1)*_othernum);
        return vec_data[index+_regionnum+1+subregionindex];
    }

    float In_flow(int timeindex,int subregionindex) const
    {
        int index=timeindex==0?0:(_headnum+(timeindex-1)*_othernum);
        return vec_data[index+2*(_regionnum+1)+subregionindex];
    }

    float h_Mean(int timeindex,int subregionindex) const
    {
        int index=timeindex==0?0:(_headnum+(timeindex-1)*_othernum);
        return vec_data[index+3*(_regionnum+1)+subregionindex];
    }

    float Top_Flux(int timeindex) const
    {
        int index=timeindex==0?0:(_headnum+(timeindex-1)*_othernum);
        return vec_data[index+4*(_regionnum+1)];
    }

    float Bot_Flux(int timeindex) const
    {
        int index=timeindex==0?0:(_headnum+(timeindex-1)*_othernum);
        return vec_data[index+4*(_regionnum+1)+1];
    }

    float WatBalT(int timeindex) const
    {
        if(timeindex)
        {
            int index=_headnum+(timeindex-1)*_othernum;
            return vec_data[index+4*(_regionnum+1)+2];
        }
        return -9999;
    }

    float WatBalR(int timeindex) const
    {
        if(timeindex)
        {
            int index=_headnum+(timeindex-1)*_othernum;
            return vec_data[index+4*(_regionnum+1)+3];
        }
        return -9999;
    }

    const std::string& DateAndTime() const
    {
        return _DateandTime;
    }

    double CalculationTime() const
    {
        return _caltime;
    }

    virtual void SaveAsBalancefFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const BalanceObject& obj);
    friend std::istream& operator>>(std::istream& in, BalanceObject& obj);
protected: 
    BalanceObject()
    {
        _headnum=0;
        _othernum=0;
    }

    void Initial();
    std::string FormatFloat(float value);
    std::string FormatLine(int timeindex, int lineindex);
    std::string FormatFile();

    int _headnum;
    int _othernum;
private:
    std::vector<std::string> vec_lineheads;
    std::vector<std::vector<std::string>> vec_linecontents;
};

#endif // BALANCEOBJECT_H
