#ifndef BALANCEOBJECT_H
#define BALANCEOBJECT_H

#include <ostream>
#include <list>
#include <memory>
#include "IHydrusParameterFileObject.h"

class QSqlQuery;
class HydrusParameterFilesManager;

class BalanceObject
{
    struct BalanceRecord
    {
        BalanceRecord(BalanceObject& parent,std::list<std::string>& part);
        BalanceRecord(const int NLayer, const int NS);
        double _Time;
        double _ATot;
        std::unique_ptr<double[]> _Area;
        double _Volume;
        std::unique_ptr<double[]> _SubVol;
        double _Change;
        std::unique_ptr<double[]> _SubCha;
        double _hTot;
        std::unique_ptr<double[]> _hMean;
        std::unique_ptr<double[]> _ConVol;
        std::unique_ptr<double[]> _ConSub;
        std::unique_ptr<double[]> _cTot;
        std::unique_ptr<double[]> _cMean;
        std::unique_ptr<double[]> _ConVolIm;
        std::unique_ptr<double[]> _ConSubIm;
        std::unique_ptr<double[]> _cTotIm;
        std::unique_ptr<double[]> _cMeanIm;
        double _Vn,_V1;
        double _wBalT,_wBalR;
        std::unique_ptr<double[]> _cBalT;
        std::unique_ptr<double[]> _cBalR;
        bool ParseArea(const char* pLine,const int NLayer);
        bool ParseW_volumn(const char* pLine,const int NLayer);
        bool ParseInflow(const char* pLine,const int NLayer);
        bool ParsehMean(const char* pLine,const int NLayer);
        bool ParseConcVol(const char* pLine, const int NLayer);
        bool ParseConcVolIm(const char* pLine,const int NLayer);
        bool ParseSorbVolIm(const char* pLine, const int NLayer);
        bool ParseSMeanIm(const char* pLine,const int NLayer);
        bool ParseCMean(const char* pLine, const int NLayer);
        bool ParseCMeanIM(const char* pLine,const int NLayer);
        bool ParseTopFlux(const char* pLine);
        bool ParseBotFlux(const char* pLine);
        bool ParseWatBalT(const char* pLine);
        bool ParseWatBalR(const char* pLine);
        bool ParseCncBalT(const char* pLine);
        bool ParseCncBalR(const char* pLine);
    };
public:
    BalanceObject(const std::string& filename,HydrusParameterFilesManager *parent);
    BalanceObject(int gid, QSqlQuery &qry,HydrusParameterFilesManager *parent);
    operator bool()
    {
        return _isValid;
    }
    virtual ~BalanceObject();
    bool Save(const std::string& path);
    std::string ToSqlStatement(const int gid);
    bool open(const std::string& filename);
    bool open(int gid,QSqlQuery& qry);
private:
    bool _isValid;
    int _NLayer;
    int _NS;
    double _CalTm;
    std::list<std::unique_ptr<BalanceRecord>> _Recs;
    HydrusParameterFilesManager *_parent;
private:
    void WriteHead(std::ostream &out);
    void WriteSection(std::ostream &out, BalanceRecord& lst, const int i);
    void WriteEnd(std::ostream &out);
    std::string ToSqlStatementPart1(const int gid);
    std::string ToSqlStatementPart2(const int gid);
    bool QueryTotalTable(int gid,QSqlQuery& qry);
    bool QueryLayerTable(int gid,QSqlQuery& qry);
    BalanceRecord* GetRecord(double tm);
};

#endif // BALANCEOBJECT_H
