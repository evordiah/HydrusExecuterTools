
/******************************************************************************
 *
 *
 *  Copyright (c) 2020, Wenzhao Feng.
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

#include <string>
#include <vector>
#include <istream>
#include <memory>
#include "IHydrusParameterFileObject.h"

namespace pqxx
{
    class connection;
}
class HydrusParameterFilesManager;
class AtmosphObject;
class ProfileObject;

class SelectorObject:public IHydrusParameterFileObject
{
public:
    SelectorObject(const std::string& filename,HydrusParameterFilesManager* parent);
    SelectorObject(int gid, pqxx::connection &qry,HydrusParameterFilesManager* parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~SelectorObject();
    bool Save(const std::string& path);
    bool Save(std::ostream& out);
    std::string ToSqlStatement( int gid);
    bool open(const std::string& filename);
    bool open(int gid,pqxx::connection& qry);
private:
    bool _isValid;
    HydrusParameterFilesManager* _parent;
    std::string _Hed;
    std::string _LUnit;
    std::string _TUnit;
    std::string _MUnit;
    std::string _status;
    bool TopInF,BotInF,ShortO,lWat,lChem,lTemp,SinkF,WLayer,qGWLF,
    FreeD,SeepF,AtmBC,lRoot,lWDep,lEquil,lScreen,qDrain,lEnter,
    lHP1,lIrrig,lInitW,lVarBC,lPrintD,lMeteo,lVapor,lCentrif,lSnow,
    lFlux,lActRSU,lDummy,lUpW,lArtD,lTDep,lTort,lFiltr,lMoistDep,
    lDualNEq,lMassIni,lEqInit,lTortA,lVar,lSolRed,lInverse;
    int NMat,NLay,MaxIt,KodTop,KodBot,iModel,iHyst,ItMin,ItMax,MPL,MaxItC,
    nPrintSteps,iRootIn,nGrowth,iBact,nChPar,iNonEqul,NS,kTopCh,kBotCh,iMoSink;
    double CosAlf,TolTh,TolH,hSeep,hTab1,hTabN,dt,dtMin,dtMax,dMul,
    dMul2,tInit,tMax,tPrintInterval,epsi,cTolA,cTolR,PeCr,dSurf,cAtm,tPulse,
    P0,P2H,P2L,P3,r2H,r2L,OmegaC;
    std::unique_ptr<double[]> _MatPars;
    std::unique_ptr<double[]> _TPrint;
    std::unique_ptr<double[]> _tGrowth;
    std::unique_ptr<double[]> _RootDepth;
    std::unique_ptr<double[]> _ChPart1;
    std::unique_ptr<double[]> _ChPart2;
    std::unique_ptr<double[]> _ChPart3;
    std::unique_ptr<double[]> _InitialTopCon;
    std::unique_ptr<double[]> _InitialBotCon;
    std::unique_ptr<double[]> _CRootMax;
    std::unique_ptr<double[]> _POptm;
    //for atmosph
    int _MaxAl;
    double _hCritS;
    //for profile
    int _NumNP;
    int _NObs;
    //for Balance
    double _CalTm;
    std::unique_ptr<int[]> _iObs;
private:
    bool ParseLine(const std::string& line,const std::string& lineformat, const std::vector<void *> &values);
    void Initial();
    char boolalpha(bool value)
    {
        return value?'t':'f';
    }
    bool ReadBlockA(std::istream& in);
    bool ReadBlockB(std::istream& in);
    bool ReadBlockC(std::istream& in);
    bool ReadBlockD(std::istream& in);
    bool ReadBlockF(std::istream& in);
    bool ReadBlockG(std::istream& in);
    bool SaveBlockA(std::ostream& out);
    bool SaveBlockB(std::ostream& out);
    bool SaveBlockC(std::ostream& out);
    bool SaveBlockD(std::ostream& out);
    bool SaveBlockF(std::ostream& out);
    bool SaveBlockG(std::ostream& out);
    std::string ToSqlArray(double *p, int nsize);
    std::string ToSqlArray(int *p, int nsize);
    bool ParseSqlARRAY(const std::string &value, double *p, int nsize);
    bool ParseSqlARRAY(const std::string &value, int *p, int nsize);
    void UpdateObsInfo();
    friend class AtmosphObject;
    friend class ProfileObject;
};

#endif // SELECTOROBJECT_H
