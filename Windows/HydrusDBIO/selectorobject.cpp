
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

#include "selectorobject.h"
#include <sstream>
#include <QDir>
#include <fstream>
#include <vector>

SelectorObject::SelectorObject()
{
}

SelectorObject::SelectorObject(std::istream& in)
{
    int dt[11];
    char unit[50];
    in.read((char*)dt,sizeof(int));
    in.read(unit,dt[0]);
    unit[dt[0]]='\0';
    _LUnit=unit;

    in.read((char*)dt,sizeof(int));
    in.read(unit,dt[0]);
    unit[dt[0]]='\0';
    _TUnit=unit;

    in.read((char*)dt,11*sizeof(int));
    _lRoot=dt[0];
    _lSink=dt[1];
    _lWlayer=dt[2];
    _lInitW=dt[3];
    _Maxit=dt[4];
    _nMat=dt[5];
    _nLay=dt[6];
    _ItMin=dt[7];
    _ItMax=dt[8];
    _printTimes=dt[9];
    _model=dt[10];

    float fltdt[19];
    in.read((char*)fltdt,19*sizeof(float));
    _TolTh=fltdt[0];
    _TolH=fltdt[1];
    _ha=fltdt[2];
    _hb=fltdt[3];
    _dt=fltdt[4];
    _dtMin=fltdt[5];
    _dtMax=fltdt[6];
    _dMul=fltdt[7];
    _dMul2=fltdt[8];
    _r2H=fltdt[9];
    _r2L=fltdt[10];
    _Omegac=fltdt[11];
    _initTime=fltdt[12];
    _maxTime=fltdt[13];
    _printInterval=fltdt[14];
    _P0=fltdt[15];
    _P2H=fltdt[16];
    _P2L=fltdt[17];
    _P3=fltdt[18];

    vec_printTimedata.reset(new float[_printTimes]);
    in.read((char*)(vec_printTimedata.get()),sizeof(float)*_printTimes);

    int spCnt=GetSPCnt();
    vec_matdata.reset(new float[_nMat*spCnt]);
    in.read((char*)(vec_matdata.get()),sizeof(float)*(_nMat*spCnt));

    vec_poptmdata.reset(new float[_nMat]);
    in.read((char*)(vec_poptmdata.get()),sizeof(float)*_nMat);

    int ncnt=0;
    in.read((char*)&ncnt,sizeof(int));
    if(!ncnt)
    {
        return;
    }
    std::unique_ptr<float[]> pndt(new float[ncnt]);
    std::unique_ptr<float[]> prootlength(new float[ncnt]);
    float* ndt=pndt.get();
    float* rootlength=prootlength.get();
    in.read((char*)ndt,sizeof(float)*ncnt);
    in.read((char*)rootlength,sizeof(float)*ncnt);
    for(int i=0; i<ncnt; i++)
    {
        _RootGrowthTable.insert(std::make_pair(ndt[i],rootlength[i]));
    }
}

SelectorObject::SelectorObject(const std::string &filename):SelectorEncoder(filename)
{

}

SelectorObject::~SelectorObject()
{
}

std::string SelectorObject::GetPrintTime()
{
    std::stringstream strbld;
    strbld<<"array[";
    for(int i=0;i<_printTimes;++i)
    {
        strbld<<vec_printTimedata[i]<<",";
    }
    std::string val=strbld.str();
    val.back()=']';
    return val;
}

std::string SelectorObject::GetPoptm()
{
    std::stringstream strbld;
    strbld<<"array[";
    for(int i=0;i<_nMat;++i)
    {
        strbld<<vec_poptmdata[i]<<",";
    }
    std::string val=strbld.str();
    val.back()=']';
    return val;
}

std::string SelectorObject::GetMatData()
{
    int spCnt=GetSPCnt();
    std::stringstream strbld;
    strbld<<"array[";
    for(int i=0;i<_nMat;++i)
    {
        for(int j=0;j<spCnt;++j)
        {
            strbld<<vec_matdata[i*spCnt+j]<<",";
        }
    }
    std::string val=strbld.str();
    val.back()=']';
    return val;
}

std::string SelectorObject::GetRootGrowthDays()
{
    if(!_lRoot)
    {
        return "";
    }
    std::stringstream strbld;
    strbld<<"array[";
    for(auto it=_RootGrowthTable.begin();it!=_RootGrowthTable.end();++it)
    {
        strbld<<it->first<<",";
    }
    std::string val=strbld.str();
    val.back()=']';
    return val;
}

std::string SelectorObject::GetRootLengthData()
{
    if(!_lRoot)
    {
        return "";
    }
    std::stringstream strbld;
    strbld<<"array[";
    for(auto it=_RootGrowthTable.begin();it!=_RootGrowthTable.end();++it)
    {
        strbld<<it->second<<",";
    }
    std::string val=strbld.str();
    val.back()=']';
    return val;
}

std::string SelectorObject::GetHead()
{
    return "Pcp_File_Version=4\n";
}

std::string SelectorObject::GetBlockA()
{
    std::stringstream strbld;
    std::string result="*** BLOCK A: BASIC INFORMATION *****************************************\n"
                       "Heading\n"
                       "Welcome to HYDRUS-1D\n"
                       "LUnit  TUnit  MUnit  (indicated units are obligatory for all input data)\n"
                       "[LUNIT]\n"
                       "[TUNIT]\n"
                       "mmol\n"
                       "lWat   lChem lTemp  lSink lRoot lShort lWDep lScreen lVariabBC lEquil lInverse\n"
                       " t     f     f      [SINK]     [ROOT]     t      f     f       t         t         f\n"
                       "lSnow  lHP1   lMeteo  lVapor lActiveU lFluxes lIrrig  lDummy  lDummy  lDummy\n"
                       " f       f       f       f       f       t       f       f       f       f\n"
                       "NMat    NLay  CosAlpha\n";
    auto pos=result.find("[LUNIT]");
    result=result.replace(pos,7,_LUnit);
    pos=result.find("[TUNIT]");
    result=result.replace(pos,7,_TUnit);
    pos=result.find("[SINK]");
    result=result.replace(pos,6,_lSink==1 ? "t":"f");
    pos=result.find("[ROOT]");
    result=result.replace(pos,6,_lRoot==1 ? "t":"f");
    strbld<<result;
    strbld.width(3);
    strbld<<_nMat;
    strbld.width(8);
    strbld<<_nLay;
    strbld.width(8);
    strbld<<1<<std::endl;
    return strbld.str();
}

std::string SelectorObject::GetBlockB()
{
    std::stringstream strbld;
    std::string result="*** BLOCK B: WATER FLOW INFORMATION ************************************\n"
                       "MaxIt   TolTh   TolH       (maximum number of iterations and tolerances)\n";
    strbld<<result;
    strbld.width(4);
    strbld<<_Maxit;
    strbld.width(9);
    strbld<<_TolTh;
    strbld.width(7);
    strbld<<_TolH;
    strbld<<std::endl;
    result="TopInf WLayer KodTop InitCond\n"
           " t     [WALYER]      -1       [INITCOND]\n"
           "BotInf qGWLF FreeD SeepF KodBot DrainF  hSeep\n"
           " f     f     t     f     -1      f      0\n"
           "    hTab1   hTabN\n";
    auto pos=result.find("[WALYER]");
    result=result.replace(pos,8,_lWlayer==1?"t":"f");
    pos=result.find("[INITCOND]");
    result=result.replace(pos,10,_lInitW==1?"t":"f");
    strbld<<result;
    strbld.width(10);
    strbld<<_ha;
    strbld.width(8);
    strbld<<_hb<<std::endl;
    strbld<<"    Model   Hysteresis\n";
    strbld.width(7);
    strbld<<_model;
    strbld.width(11);
    strbld<<0<<std::endl;
    strbld<<"   thr     ths    Alfa      n         Ks       l\n";
    int spCnt=GetSPCnt();
    for(int i=0; i<_nMat; ++i)
    {
        strbld.width(9);
        strbld<<vec_matdata[i*spCnt];
        for(int j=1; j<4; ++j)
        {
            strbld.width(12);
            strbld<<vec_matdata[i*spCnt+j];
        }
        strbld.width(11);
        strbld<<vec_matdata[i*spCnt+4];
        strbld.width(9);
        strbld<<vec_matdata[i*spCnt+5];
        if(spCnt==10)
        {
            for(int j=6; j<10; ++j)
            {
                strbld.width(9);
                strbld<<vec_matdata[i*spCnt+j];
            }
        }
        strbld<<" \n";
    }
    return strbld.str();
}

std::string SelectorObject::GetBlockC()
{
    std::stringstream strbld;
    std::string result="*** BLOCK C: TIME INFORMATION ******************************************\n"
                       "        dt       dtMin       dtMax     DMul    DMul2  ItMin ItMax  MPL\n";
    strbld<<result;
    strbld.width(11);
    strbld<<_dt;
    strbld.width(12);
    strbld<<_dtMin;
    strbld.width(12);
    strbld<<_dtMax;
    strbld.width(8);
    strbld<<_dMul;
    strbld.width(8);
    strbld<<_dMul2;
    strbld.width(6);
    strbld<<_ItMin;
    strbld.width(6);
    strbld<<_ItMax;
    strbld.width(6);
    strbld<<_printTimes<<std::endl;
    strbld<<"      tInit        tMax\n";
    strbld.width(11);
    strbld<<_initTime;
    strbld.width(12);
    strbld<<_maxTime<<std::endl;
    strbld<<"  lPrintD  nPrintSteps tPrintInterval lEnter\n";
    strbld.width(6);
    strbld<<'t';
    strbld.width(12);
    strbld<<1;
    strbld.width(14);
    strbld<<_printInterval;
    strbld.width(8);
    strbld<<'f'<<std::endl;
    strbld<<"TPrint(1),TPrint(2),...,TPrint(MPL)\n";

    for(int i=0; i<_printTimes; ++i)
    {
        if(i%6==0)
        {
            strbld.width(11);
        }
        else
        {
            strbld.width(12);
        }
        strbld<<vec_printTimedata[i];
        if(i%6==5)
        {
            strbld<<std::endl;
        }
    }
    std::string val=strbld.str();
    if(val.back()!='\n')
    {
        val.push_back('\n');
    }
    return val;
}

std::string SelectorObject::GetBlockD()
{
    std::stringstream strbld;
    std::string result="*** BLOCK D: ROOT GROWTH INFORMATION ***********************************\n"
                       "iRootDepthEntry\n"
                       "        1\n"
                       " nGrowth\n";
    strbld<<result;
    strbld.width(8);
    strbld<<_RootGrowthTable.size()<<std::endl;
    strbld<<"      Time  RootDepth\n";
    for(auto it=_RootGrowthTable.begin(); it!=_RootGrowthTable.end(); ++it)
    {
        strbld.width(10);
        strbld<<it->first;
        strbld.width(11);
        strbld<<it->second;
        strbld<<" \n";
    }
    return strbld.str();
}

std::string SelectorObject::GetBlockG()
{
    std::stringstream strbld;
    std::string result="*** BLOCK G: ROOT WATER UPTAKE INFORMATION *****************************\n"
                       "     Model  (0 - Feddes, 1 - S shape)  cRootMax    OmegaC\n";
    strbld<<result;
    strbld.width(9);
    strbld<<0;
    strbld.width(36);
    strbld<<_Omegac<<std::endl;
    strbld<<"       P0       P2H       P2L       P3          r2H        r2L\n";
    strbld.width(9);
    strbld<<_P0;
    strbld.width(10);
    strbld<<_P2H;
    strbld.width(10);
    strbld<<_P2L;
    strbld.width(10);
    strbld<<_P3;
    strbld.width(12);
    strbld<<_r2H;
    strbld.width(12);
    strbld<<_r2L<<std::endl;
    strbld<<"POptm(1),POptm(2),...,POptm(NMat)\n";
    strbld.width(8);
    strbld<<-1;
    for(int i=1;i<_nMat;++i)
    {
        strbld.width(9);
        strbld<<-1;
    }
    strbld<<std::endl;
    return strbld.str();
}

std::string SelectorObject::GetEnd()
{
    return "*** END OF INPUT FILE 'SELECTOR.IN' ************************************\n";
}

void SelectorObject::SaveAsSelectorFile(const std::string& strpath)
{
	QDir p(strpath.c_str());
	if (!p.exists())
	{
		p.mkpath(strpath.c_str());
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("SELECTOR.IN")).toStdString();
    std::ofstream out(file);
    out<<GetHead();
    out<<GetBlockA();
    out<<GetBlockB();
    out<<GetBlockC();
    if(_lRoot)
    {
        out<<GetBlockD();
    }
    if(_lSink)
    {
        out<<GetBlockG();
    }
    out<<GetEnd();
    out.close();
}

std::istream& operator>>(std::istream &in, SelectorObject &obj)
{
    obj.ParseFile(in);
    return in;
}

std::ostream& operator<<(std::ostream& out,const SelectorObject& obj)
{
    out<<(SelectorEncoder&)(obj);
    return out;
}
