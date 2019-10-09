
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

#ifndef SELECTORPARSE_H
#define SELECTORPARSE_H
#include <string>
#include <map>
#include <iostream>
#include <memory>
#include <vector>

class SelectorEncoder
{
    public:
        SelectorEncoder(const std::string& filename);
        virtual ~SelectorEncoder(){}
        //return the whole size of file,including head
        unsigned int GetLength() const
        {
            int cnt=0;
            cnt+=_LUnit.size();
            cnt+=_TUnit.size();
            cnt+=sizeof(int)*(2+11);
            cnt+=sizeof(float)*19;
            cnt+=sizeof(float)*_nMat*(_model==1 ?10:6);
            cnt+=sizeof(float)*_printTimes;
            cnt+=sizeof(float)*_nMat;
            cnt+=sizeof(int);
            int rootcnt=_RootGrowthTable.size();
            cnt+=2*rootcnt*sizeof(float);
            return cnt;
        }
        bool ParseFile(const std::string& filename);
        friend std::ostream& operator<<(std::ostream& out,const SelectorEncoder& SEncoder);
    protected:
        SelectorEncoder();
        std::string _LUnit,_TUnit;
        int _lRoot,_lSink,_lWlayer,_lInitW;
        int _Maxit,_nMat,_nLay,_ItMin,_ItMax,_printTimes,_model;
        float _TolTh,_TolH,_ha,_hb,_dt,_dtMin,_dtMax,_dMul,_dMul2,_initTime,_maxTime,
              _printInterval,_P0,_P2H,_P2L,_P3,_r2H,_r2L,_Omegac;
        std::unique_ptr<float[]> vec_matdata;
        std::unique_ptr<float[]> vec_printTimedata;
        std::unique_ptr<float[]> vec_poptmdata;
        std::map<float,float> _RootGrowthTable;
        std::map<std::string,bool> _logicoptions;
        std::map<std::string,int> _intoptions;
        std::map<std::string,float> _fltoptions;
        std::map<std::string,std::string> _stroptions;
        std::string cleanString(const std::string &val);
        std::vector<bool> GetLogicalOptions(const std::string &val);
        bool GetLogicalAndNumericOptions(const std::string &val, std::vector<bool> &bvals, std::vector<float> &nvals);
        bool ParseFile(std::istream& in);
private:
        bool ParseBlockA(std::istream& in);
        bool ParseBlockB(std::istream& in);
        bool ParseBlockC(std::istream& in);
        bool ParseBlockD(std::istream& in);
        bool ParseBlockG(std::istream& in);
};

#endif // SELECTORPARSE_H
