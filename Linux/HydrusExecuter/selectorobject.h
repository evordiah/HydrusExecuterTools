
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

#ifndef SELECTOROBJECT_H
#define SELECTOROBJECT_H
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "SelectorParse.h"

class SelectorObject:public SelectorEncoder
{
public:
    enum Model
    {
        VGM=0,
        mVGM=1,
        BC=2,
        VGM_2=3,
        Kosogi=4
    };
    SelectorObject();
    SelectorObject(std::istream& in);
    SelectorObject(const std::string& filename);
    virtual ~SelectorObject();

    std::string LUnit()
    {
        return _LUnit;
    }
    void LUnit(const std::string& val)
    {
        _LUnit=val;
    }

    std::string TUnit()
    {
        return _TUnit;
    }
    void TUnit(const std::string& val)
    {
        _TUnit=val;
    }

    bool LRoot()
    {
        return _lRoot==1 ? true : false;
    }
    void LRoot(bool value)
    {
        _lRoot=value ? 1 : 0;
    }

    bool LSink()
    {
        return _lSink==1 ? true : false;
    }
    void LSink(bool value)
    {
        _lSink=value ? 1 : 0;
    }

    bool LWlayer()
    {
        return _lWlayer==1 ? true : false;
    }
    void LWlayer(bool value)
    {
        _lWlayer=value ? 1 : 0;
    }

    bool LInitW()
    {
        return _lInitW==1 ? true : false;
    }
    void LInitW(bool value)
    {
        _lInitW=value ? 1 : 0;
    }

    int Maxit()
    {
        return _Maxit;
    }
    void Maxit(int value)
    {
        _Maxit=value;
    }

    int ItMin()
    {
        return _ItMin;
    }
    void ItMin(int value)
    {
        _ItMin=value;
    }

    int ItMax()
    {
        return _ItMax;
    }
    void ItMax(int value)
    {
        _ItMax=value;
    }

    float InitTime()
    {
        return _initTime;
    }
    void InitTime(float value)
    {
        _initTime=value;
    }

    float TolTh()
    {
        return _TolTh;
    }
    void TolTh(float value)
    {
        _TolTh=value;
    }

    float TolH()
    {
        return _TolH;
    }
    void TolH(float value)
    {
        _TolH=value;
    }

    float HA()
    {
        return _ha;
    }
    void HA(float value)
    {
        _ha=value;
    }

    float HB()
    {
        return _hb;
    }
    void HB(float value)
    {
        _hb=value;
    }

    float DT()
    {
        return _dt;
    }
    void DT(float value)
    {
        _dt=value;
    }

    float DtMin()
    {
        return _dtMin;
    }
    void DtMin(float value)
    {
        _dtMin=value;
    }

    float DtMax()
    {
        return _dtMax;
    }
    void DtMax(float value)
    {
        _dtMax=value;
    }

    float DMul()
    {
        return _dMul;
    }
    void DMul(float value)
    {
        _dMul=value;
    }

    float DMul2()
    {
        return _dMul2;
    }
    void DMul2(float value)
    {
        _dMul2=value;
    }

    void nMat(int val)
    {
        if(val<=0)
        {
            val=1;
        }
        if(_nMat==val)
        {
            return;
        }
        _nMat=val;
        vec_matdata.reset(new float[val*GetSPCnt()]);
        vec_poptmdata.reset(new float[val]);
    }
    int nMat()
    {
        return _nMat;
    }

    void nLay(int val)
    {
        _nLay=val;
    }
    int nLay()
    {
        return _nLay;
    }

    void PrintInterval(float val)
    {
        _printInterval=val;
    }
    float PrintInterval()
    {
        return _printInterval;
    }

    void MaxTime(float val)
    {
        _maxTime=val;
    }
    float MaxTime()
    {
        return _maxTime;
    }

    void PrintTimes(int val)
    {
        if(val<=0)
        {
            val=1;
        }
        if(_printTimes==val)
        {
            return;
        }
        _printTimes=val;
        vec_printTimedata.reset(new float[val]);
    }
    int PrintTimes()
    {
        return _printTimes;
    }

    void SetPrintTime(int idx,float val)
    {
        vec_printTimedata[idx]=val;
    }

    float GetPrintTime(int idx)
    {
        return vec_printTimedata[idx];
    }

    std::string GetPrintTime();

    void SetPoptm(int idx,float val)
    {
        vec_poptmdata[idx]=val;
    }

    float GetPoptm(int idx)
    {
        return vec_poptmdata[idx];
    }

    std::string GetPoptm();

    void SetMatThetaRData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()]=val;
    }

    float GetMatThetaRData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()];
    }

    void SetMatThetaSData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+1]=val;
    }

    float GetMatThetaSData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+1];
    }
    void SetMatAlfaData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+2]=val;
    }
    float GetMatAlfaData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+2];
    }

    void SetMatNData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+3]=val;
    }
    float GetMatNData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+3];
    }

    void SetMatKsData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+4]=val;
    }
    float GetMatKsData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+4];
    }

    void SetMatLData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+5]=val;
    }
    float GetMatLData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+5];
    }
    //only for model=1
    void SetMatThetamData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+6]=val;
    }
    float GetMatThetamData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+6];
    }
    //only for model=1
    void SetMatThetaalphaData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+7]=val;
    }
    float GetMatThetaalphaData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+7];
    }

    //only for model=1
    void SetMatThetaKData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+8]=val;
    }
    float GetMatThetaKData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+8];
    }

    //only for model=1
    void SetMatKKData(int idx,float val)
    {
        vec_matdata[(idx-1)*GetSPCnt()+9]=val;
    }
    float GetMatKKData(int idx)
    {
        return vec_matdata[(idx-1)*GetSPCnt()+9];
    }

    std::string GetMatData();

    void P0(float val)
    {
        _P0=val;
    }
    float P0()
    {
        return _P0;
    }

    void P2H(float val)
    {
        _P2H=val;
    }
    float P2H()
    {
        return _P2H;
    }

    void P2L(float val)
    {
        _P2L=val;
    }
    float P2L()
    {
        return _P2L;
    }

    void P3(float val)
    {
        _P3=val;
    }
    float P3()
    {
        return _P3;
    }

    void SPModel(Model val)
    {
        _model=(int)val;
    }
    Model SPModel()
    {
        return (Model)_model;
    }

    void Omegac(float val)
    {
        _Omegac=val;
    }
    float Omegac()
    {
        return _Omegac;
    }

    void r2H(float val)
    {
        _r2H=val;
    }
    float r2H()
    {
        return _r2H;
    }

    void r2L(float val)
    {
        _r2L=val;
    }
    float r2L()
    {
        return _r2L;
    }

    void SetRootGrowth(float dseq,float rootLength)
    {
        _RootGrowthTable[dseq]=rootLength;
    }
    int GetRootGrowthCnt()
    {
        return _RootGrowthTable.size();
    }
    std::string GetRootGrowthDays();
    std::string GetRootLengthData();

    void SaveAsSelectorFile(const std::string& path);
    friend std::ostream& operator<<(std::ostream& out,const SelectorObject& obj);
    friend std::istream& operator>>(std::istream& in, SelectorObject& obj);
protected:
    int GetSPCnt() const
    {
        return _model==1 ? 10:6;
    }
    std::string GetHead();
    std::string GetBlockA();
    std::string GetBlockB();
    std::string GetBlockC();
    std::string GetBlockD();
    std::string GetBlockG();
    std::string GetEnd();
};

#endif // SELECTOROBJECT_H
