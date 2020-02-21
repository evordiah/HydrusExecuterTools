#include <QCoreApplication>
#include <iostream>
#include <fstream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "HydrusParameterFilesManager.h"


int main(int argc, char *argv[])
{
    using namespace std;
    QCoreApplication app(argc, argv);
    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL","justtest");
    db.setDatabaseName("byd_hydrus_db");
    db.setHostName("localhost");
    db.setUserName("postgres");
    db.setPassword("123");
    db.open();
    QSqlQuery qry(db);
    HydrusParameterFilesManager h(65537,"/tmp/lx",qry);
    //    if(h.ImportInputFiles())
    //    {
    //        std::cout<<"import input files successfully"<<std::endl;
    //    }
    //    else
    //    {
    //        std::cout<<"import input files failed"<<std::endl;
    //    }
    if(h.ExportInputFiles())
    {
        std::cout<<"export input files successfully"<<std::endl;
    }
    else
    {
        std::cout<<"export input files failed"<<std::endl;
    }
    auto r=h.GetImportResultFilesSQlStatement();
    if(r)
    {
        ofstream out("/tmp/lx/isql.v");
        out<<*r;
        out.close();
        std::cout<<"import result files successfully"<<std::endl;
    }
    else
    {
        std::cout<<"import result files failed"<<std::endl;
    }
//    if(h.ExportResultFiles())
//    {
//        std::cout<<"export result files successfully"<<std::endl;
//    }
//    else
//    {
//        std::cout<<"export result files failed"<<std::endl;
//    }
    return app.exec();
}
