
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

#include "SelectorParse.h"
#include <sstream>
#include <vector>
#include <string.h>
#include <limits>
#include <QFileInfo>
#include <fstream>

//In hydrus SELECTOR.IN controls all the options. And there are too many options to
//deal with, and the program only address the common case, which considered
//water flow without solute transport, the root uptake water ,root growth,
//using variable boundary condition
SelectorEncoder::SelectorEncoder(const std::string &filename)
{
    if(!ParseFile(filename))
    {
        throw std::string("Can not parse file: ")+filename;
    }
}

bool SelectorEncoder::ParseFile(const std::string &filename)
{
    using namespace std;
	if(!QFileInfo::exists(filename.c_str()))
    {
        cout<<filename<<" does not exist!"<<endl;
        return false;
    }
    ifstream in(filename);
    if(!ParseFile(in))
    {
        cout<<"can not parse file: "<<filename<<std::endl;
        return false;
    }
    return true;
}

SelectorEncoder::SelectorEncoder()
{
    _LUnit="cm";
    _TUnit="days";
    _lRoot=1;
    _lSink=1;
    _lWlayer=1;
    _lInitW=1;
    _Maxit=500;
    _nMat=0;
    _nLay=0;
    _ItMin=3;
    _ItMax=7;
    _initTime=0;
    _maxTime=1;
    _printInterval=10;
    _P0=0;
    _P2H=-500;
    _P2L=-900;
    _P3=-16000;
    _printTimes=1;
    _model=4;
    _TolTh=0.001;
    _TolH=1;
    _ha=1e-6;
    _hb=10000;
    _dt=0.001;
    _dtMin=1e-5;
    _dtMax=0.5;
    _dMul=1.3;
    _dMul2=0.7;
    _r2H=0.5;
    _r2L=0.1;
    _Omegac=1;
}

std::ostream & operator<<(std::ostream &out, const SelectorEncoder &obj)
{
    //save Lunit and Tunit
    int tmpi=obj._LUnit.size();
    out.write((const char*)(&tmpi),sizeof(int));
    out.write(obj._LUnit.c_str(),tmpi);
    tmpi=obj._TUnit.size();
    out.write((const char*)(&tmpi),sizeof(int));
    out.write(obj._TUnit.c_str(),tmpi);
    //save integer data
    int dt[11]= {obj._lRoot,obj._lSink,obj._lWlayer,obj._lInitW,
                 obj._Maxit,obj._nMat,obj._nLay,
                 obj._ItMin,obj._ItMax,obj._printTimes,obj._model};
    //save float data
    out.write((const char*)dt,11*sizeof(int));
    float flt[19]={obj._TolTh,obj._TolH,obj._ha,obj._hb,
                   obj._dt,obj._dtMin,obj._dtMax,obj._dMul,
                   obj._dMul2,obj._r2H,obj._r2L,obj._Omegac,
                   obj._initTime,obj._maxTime,obj._printInterval,
                   obj._P0,obj._P2H,obj._P2L,obj._P3};
    out.write((const char*)flt,19*sizeof(float));
    //save printtime
    out.write((const char*)obj.vec_printTimedata.get(),sizeof(float)*obj._printTimes);
    //save matdata
    out.write((const char*)obj.vec_matdata.get(),sizeof(float)*obj._nMat*(obj._model ==1?10:6));
    //save vec_poptmdata
    out.write((const char*)obj.vec_poptmdata.get(),sizeof(float)*obj._nMat);
    //save root growth data
    int rootcnt=obj._RootGrowthTable.size();
    out.write((const char*)(&rootcnt),sizeof(int));
    if(!rootcnt)
    {
        return out;
    }
    std::unique_ptr<float[]> pftmp(new float[rootcnt]);
    std::unique_ptr<float[]> pftmp2(new float[rootcnt]);
    int ind=0;
    for(auto it=obj._RootGrowthTable.begin(); it!=obj._RootGrowthTable.end(); ++it)
    {
        pftmp[ind]=it->first;
        pftmp2[ind]=it->second;
        ind++;
    }
    out.write((const char*)pftmp.get(),sizeof(float)*rootcnt);
    out.write((const char*)pftmp2.get(),sizeof(float)*rootcnt);
    return out;
}

std::string SelectorEncoder::cleanString(const std::string& val)
{
    std::string strval=val;
    if(strval.back()=='\r')
    {
        strval.pop_back();
    }    
    auto spos=strval.find_first_not_of(' ');
    auto epos=strval.find_last_not_of(' ');
    if(spos==std::string::npos)
    {
        return "";
    }
    return strval.substr(spos,epos-spos+1);
}

std::vector<bool>  SelectorEncoder::GetLogicalOptions(const std::string& val)
{
    std::vector<bool> bVals;
    for(auto c : val)
    {
        if(c=='t' || c=='T')
        {
            bVals.push_back(true);
        }
        else if(c=='f' || c=='F')
        {
            bVals.push_back(false);
        }
    }
    return bVals;
}

bool  SelectorEncoder::GetLogicalAndNumericOptions(const std::string& val,std::vector<bool>& bvals,std::vector<float>& nvals)
{
    try
    {
        for(unsigned int i=0;i<val.size();++i)
        {
            auto c=val[i];
            if(c=='t' || c=='T')
            {
                bvals.push_back(true);
            }
            else if(c=='f' || c=='F')
            {
                bvals.push_back(false);
            }
            else if(c==' ' || c=='\t' || c=='\r')
            {
                continue;
            }
            else
            {
                std::size_t idx;
                float t=stof(val.substr(i),&idx);
                nvals.push_back(t);
                i+=(idx-1);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        return false;
    }

    return true;
}

bool SelectorEncoder::ParseFile(std::istream &in)
{
    if(!ParseBlockA(in))
    {
        return false;
    }
    if(!ParseBlockB(in))
    {
        return false;
    }
    if(!ParseBlockC(in))
    {
        return false;
    }
    if(_logicoptions["lRoot"])
    {
        if(!ParseBlockD(in))
        {
            return false;
        }
    }
    if(_logicoptions["lSink"])
    {
        if(!ParseBlockG(in))
        {
            return false;
        }
    }
    //record the necessary options for run hydrus in common cases.
    //I do't consider solute transport ,so only record LUnit and TUnit
    _LUnit=_stroptions["LUnit"];
    _TUnit=_stroptions["TUnit"];
    //the lWat=t   lChem=f lTemp=f  lShort=t lWDep=f lScreen=f AtmInf=t lEquil=t lInverse=f will be constant
    //i only consider lroot and lsink
    _lRoot=_logicoptions["lRoot"]?1:0;
    _lSink=_logicoptions["lSink"]?1:0;
    //the lSnow=f  lHP1=f   lMeteo=f  lVapor=f lActiveU=f lFluxes=t lIrrig=f will be constant
    _nMat=_intoptions["NMat"];
    _nLay=_intoptions["NLay"];
    //CosAlfa will be 1, i only consider the vertical flow
    _Maxit=_intoptions["MaxIt"];
    _TolTh=_fltoptions["TolTh"];
    _TolH=_fltoptions["TolH"];
    //TopInf=t and KodTop=-1
    _lWlayer=_logicoptions["WLayer"] ? 1:0;
    _lInitW=_logicoptions["lInitW"] ? 1:0;
    //BotInf=f qGWLF=f FreeD=t SeepF=f KodBot=-1 DrainF=f  hSeep=0 will be const
    _ha=_fltoptions["ha"];
    _hb=_fltoptions["hb"];
    _model=_intoptions["iModel"];
    //iHyst=0
    _dt=_fltoptions["dt"];
    _dtMin=_fltoptions["dtMin"];
    _dtMax=_fltoptions["dtMax"];
    _dMul=_fltoptions["DMul"];
    _dMul2=_fltoptions["DMul2"];
    _ItMin=_intoptions["ItMin"];
    _ItMax=_intoptions["ItMax"];
    _printTimes=_intoptions["MPL"];
    _initTime=_fltoptions["tInit"];
    _maxTime=_fltoptions["tMax"];
    //lPrintD=t , nPrintSteps=1,lEnter=f
    _printInterval=_fltoptions["tPrintInterval"];
    //iRootIn=1
    //iMoSink=0
    _Omegac=_fltoptions["OmegaC"];
    _P0=_fltoptions["P0"];
    _P2H=_fltoptions["P2H"];
    _P2L=_fltoptions["P2L"];
    _P3=_fltoptions["P3"];
    _r2H=_fltoptions["r2H"];
    _r2L=_fltoptions["r2L"];
    return true;
}

bool SelectorEncoder::ParseBlockA(std::istream &in)
{
    std::string line;
    //ignore first 5 lines
    for(int i=0;i<5;++i)
    {
        in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    //get LUnit
    getline(in,line);
    _stroptions["LUnit"]=cleanString(line);
    //get TUnit
    getline(in,line);
    _stroptions["TUnit"]=cleanString(line);
    //get MUnit
    getline(in,line);
    _stroptions["MUnit"]=cleanString(line);

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    auto bVals=GetLogicalOptions(line);
    if(bVals.size()!=11)
    {
        return false;
    }
    _logicoptions["lWat"]=bVals[0];
    _logicoptions["lChem"]=bVals[1];
    _logicoptions["lTemp"]=bVals[2];
    _logicoptions["lSink"]=bVals[3];
    _logicoptions["lRoot"]=bVals[4];
    _logicoptions["lShort"]=bVals[5];
    _logicoptions["lWDep"]=bVals[6];
    _logicoptions["lScreen"]=bVals[7];
    _logicoptions["AtmInf"]=bVals[8];
    _logicoptions["lEquil"]=bVals[9];
    _logicoptions["lInverse"]=bVals[10];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    bVals=GetLogicalOptions(line);
    if(bVals.size()<7)
    {
        return false;
    }
    _logicoptions["lSnow"]=bVals[0];
    _logicoptions["lHP1"]=bVals[1];
    _logicoptions["lMeteo"]=bVals[2];
    _logicoptions["lVapor"]=bVals[3];
    _logicoptions["lActiveU"]=bVals[4];
    _logicoptions["lFluxes"]=bVals[5];
    _logicoptions["lIrrig"]=bVals[6];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    std::stringstream strbld;
    strbld.str(line);
    int tmpi;
    strbld>>tmpi;
    _intoptions["NMat"]=tmpi;
    strbld>>tmpi;
    _intoptions["NLay"]=tmpi;
    float cosalfa;
    strbld>>cosalfa;
    _fltoptions["CosAlfa"]=cosalfa;
    return true;
}


bool SelectorEncoder::ParseBlockB(std::istream &in)
{
    std::string line;
    std::stringstream strbld;
    //ignore first 2 lines in BLOCK B
    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    strbld.str(line);
    int tmpi;
    strbld>>tmpi;
    _intoptions["MaxIt"]=tmpi;
    float tmpf;
    strbld>>tmpf;
    _fltoptions["TolTh"]=tmpf;
    strbld>>tmpf;
    _fltoptions["TolH"]=tmpf;

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    std::vector<bool> bVals;
    std::vector<float> nVals;
    if(!GetLogicalAndNumericOptions(line,bVals,nVals))
    {
        return false;
    }
    _logicoptions["TopInf"]=bVals[0];
    _logicoptions["WLayer"]=bVals[1];
    _logicoptions["lInitW"]=bVals[2];
    _intoptions["KodTop"]=(int)nVals[0];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    bVals.clear();
    nVals.clear();
    getline(in,line);
    if(!GetLogicalAndNumericOptions(line,bVals,nVals))
    {
        return false;
    }
    _logicoptions["BotInf"]=bVals[0];
    _logicoptions["qGWLF"]=bVals[1];
    _logicoptions["FreeD"]=bVals[2];
    _logicoptions["SeepF"]=bVals[3];
    _logicoptions["qDrain"]=bVals[4];
    _intoptions["KodBot"]=(int)nVals[0];
    _fltoptions["hSeep"]=nVals[1];

    //check some options and i do't consider such as auto irrigation, time independent surface boundary conditions,
    //time dependent boundary condition at the bottom of the  profile, groundwater level, seepage face and horizontal drains
    //the consideration is suit for commom cases
    if(!_logicoptions["TopInf"] || _logicoptions["BotInf"] || _logicoptions["qGWLF"] ||
            _logicoptions["SeepF"] || _logicoptions["qDrain"] || _logicoptions["lIrrig"])
    {
        std::cout<<"SELECTOR.IN some options not suit for this program. exit";
        return false;
    }

    while(getline(in,line))
    {
        if(line.find("hTab1")!=std::string::npos)
        {
            break;
        }
    }

    getline(in,line);
    strbld.clear();
    strbld.str(line);
    strbld>>tmpf;
    _fltoptions["ha"]=tmpf;
    strbld>>tmpf;
    _fltoptions["hb"]=tmpf;

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    strbld.clear();
    strbld.str(line);
    strbld>>tmpi;
    _intoptions["iModel"]=tmpi;
    strbld>>tmpi;
    _intoptions["iHyst"]=tmpi;

    //now the program only support model<5 and iHyst==0

    if(_intoptions["iModel"]>5 || _intoptions["iHyst"]!=0)
    {
        std::cout<<"SELECTOR.IN Soil hydraulic properties model not suit for this program. exit";
        return false;
    }

    int parcnt=_intoptions["iModel"]==1 ? 10 :6;
    vec_matdata.reset(new float[parcnt*_intoptions["NMat"]]);

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    //float fltmatval[10];
    float *pmat=vec_matdata.get();
    for(int i=0;i<_intoptions["NMat"];++i)
    {
        getline(in,line);
        strbld.clear();
        strbld.str(line);
        for(int j=0;j<parcnt;++j)
        {
            strbld>>pmat[j];
        }
        //::memcpy(pmat,fltmatval,sizeof(float)*parcnt);
        pmat+=parcnt;
    }
    return true;
}

bool SelectorEncoder::ParseBlockC(std::istream &in)
{
    std::string line;
    std::stringstream strbld;

    while(getline(in,line))
    {
        if(line.find("BLOCK C")!=std::string::npos)
        {
            break;
        }
    }

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    float fltval[8];
    getline(in,line);
    strbld.str(line);
    for(int j=0;j<8;++j)
    {
        strbld>>fltval[j];
    }
    _fltoptions["dt"]=fltval[0];
    _fltoptions["dtMin"]=fltval[1];
    _fltoptions["dtMax"]=fltval[2];
    _fltoptions["DMul"]=fltval[3];
    _fltoptions["DMul2"]=fltval[4];
    _intoptions["ItMin"]=(int)fltval[5];
    _intoptions["ItMax"]=(int)fltval[6];
    _intoptions["MPL"]=(int)fltval[7];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    strbld.clear();
    strbld.str(line);
    for(int j=0;j<2;++j)
    {
        strbld>>fltval[j];
    }
    _fltoptions["tInit"]=fltval[0];
    _fltoptions["tMax"]=fltval[1];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    std::vector<bool> bVals;
    std::vector<float> nVals;
    if(!GetLogicalAndNumericOptions(line,bVals,nVals))
    {
        return false;
    }
    _logicoptions["lPrint"]=bVals[0];
    _logicoptions["lEnter"]=bVals[1];
    _intoptions["nPrintSteps"]=(int)nVals[0];
    _fltoptions["tPrintInterval"]=nVals[1];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    int cnt=_intoptions["MPL"];
    vec_printTimedata.reset(new float[cnt]);
    float *pfltv=vec_printTimedata.get();

    int i=0;
    while(i<cnt)
    {
        if(i%6==0)
        {
            getline(in,line);
            strbld.clear();
            strbld.str(line);
        }
        strbld>>pfltv[i++];
    }
    return true;
}

bool SelectorEncoder::ParseBlockD(std::istream &in)
{
    std::string line;
    std::stringstream strbld;

    while(getline(in,line))
    {
        if(line.find("BLOCK D")!=std::string::npos)
        {
            break;
        }
    }

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    int tmpi;
    getline(in,line);
    strbld.str(line);
    strbld>>tmpi;
    _intoptions["iRootIn"]=tmpi;

    if(tmpi!=1)
    {
        std::cout<<"SELECTOR.IN rootgrowth infomation is only supported by  a table of "
                   "RootDepth values in this program now. exit";
        return false;
    }

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    strbld.clear();
    strbld.str(line);
    strbld>>tmpi;
    _intoptions["nGrowth"]=tmpi;

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    int cnt=tmpi;
    float tmpf[2];
    for(int i=0;i<cnt;++i)
    {
        getline(in,line);
        strbld.clear();
        strbld.str(line);
        strbld>>tmpf[0]>>tmpf[1];
        _RootGrowthTable[tmpf[0]]=tmpf[1];
    }
    return true;
}

bool SelectorEncoder::ParseBlockG(std::istream &in)
{
    std::string line;
    std::stringstream strbld;

    while(getline(in,line))
    {
        if(line.find("BLOCK G")!=std::string::npos)
        {
            break;
        }
    }

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    int tmpi;
    float tmpf,lasttmpf;
    getline(in,line);
    strbld.str(line);
    strbld>>tmpi;
    _intoptions["iMoSink"]=tmpi;

    while(strbld>>tmpf)
    {
        lasttmpf=tmpf;
    }
    _fltoptions["OmegaC"]=lasttmpf;

    if(tmpi!=0)
    {
        std::cout<<"SELECTOR.IN. Type of root water uptake stress response function only supported "
                   "Feddes now. exit";
        return false;
    }

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    getline(in,line);
    strbld.clear();
    strbld.str(line);
    float fltval[6];
    for(int i=0;i<6;++i)
    {
        strbld>>fltval[i];
    }
    _fltoptions["P0"]=fltval[0];
    _fltoptions["P2H"]=fltval[1];
    _fltoptions["P2L"]=fltval[2];
    _fltoptions["P3"]=fltval[3];
    _fltoptions["r2H"]=fltval[4];
    _fltoptions["r2L"]=fltval[5];

    in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    vec_poptmdata.reset(new float[_intoptions["NMat"]]);
    float *ppopt=vec_poptmdata.get();
    getline(in,line);
    strbld.clear();
    strbld.str(line);

    for(int i=0;i<_intoptions["NMat"];++i)
    {
        strbld>>ppopt[i];
    }

    return true;
}




