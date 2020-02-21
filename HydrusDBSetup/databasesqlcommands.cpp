
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

#include "databasesqlcommands.h"
#include <sstream>
#include <iostream>
#include <limits>
#include <QFile>
#include <QTextStream>

DataBaseSQLCommands::DataBaseSQLCommands()
    :_bPartitionTable(false),_tablecnt1(0),_idcnt1(0),_tablecnt2(0),_idcnt2(0),_maxid(0)
{
}

DataBaseSQLCommands::DataBaseSQLCommands(const int tablecount, const int gidcount)
    :_tablecnt1(tablecount),_idcnt1(gidcount),_idcnt2(1000),_maxid(std::numeric_limits<int>::max()-1)
{
    _bPartitionTable=tablecount>1?true:false;
    _tablecnt2=int(tablecount*gidcount/1000.0)+1;
}

std::string DataBaseSQLCommands::GetCreateDbSqlCommand(const std::string& dbname)
{
    QFile f(":/dbtemplate/cdbtemplate/dbtemplate");
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            if(line.back()==';')
            {
                cmd.append(line);
                break;
            }
            else
            {
                cmd.append(line).append("\n");
            }
        }
        return cmd.arg(dbname.c_str()).toStdString();
    }
    return "";
}

std::string DataBaseSQLCommands::GetCreateTablesSqlCommand()
{
    std::stringstream strbld;
    strbld<<"BEGIN TRANSACTION;"<<std::endl;
    std::string sqlCmd=CreateSelector();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateAtmosph();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateProfile();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateALevel();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateTLevel();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateNodeinfo();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateObsNode();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateSolute();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    sqlCmd=CreateFunction();
    if(sqlCmd.empty())
    {
        return "";
    }
    strbld<<sqlCmd;
    strbld<<"COMMIT;";
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateSelector()
{
    QFile f(":/dbtemplate/cdbtemplate/selectortemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    for(unsigned int i=0;i<Cmds.size();++i)
    {
        strbld<<Cmds[i].toStdString();
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateFunction()
{
    QFile f(":/dbtemplate/cdbtemplate/functiontemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    for(unsigned int i=0;i<Cmds.size();++i)
    {
        strbld<<Cmds[i].toStdString();
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateALevel()
{
    QFile f(":/dbtemplate/cdbtemplate/aleveltemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table a_level and index
    strbld<<Cmds[0].toStdString()
            <<Cmds[1].toStdString();
    if(!_bPartitionTable)
    {
        return strbld.str();
    }
    std::stringstream strbld1;
    int startnum;
    int endnum;
    //template for a_level partition table and index
    for(int i=1;i<=_tablecnt1;++i)
    {
        startnum=(i-1)*_idcnt1+1;
        if(i==_tablecnt1)
        {
            endnum=_maxid;
        }
        else
        {
            endnum=i*_idcnt1;
        }
        if(i==1)
        {
            strbld1<<Cmds[4].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        else
        {
            strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        strbld<<Cmds[2].arg(i).arg(i).arg(i).arg(startnum).arg(endnum).toStdString();
        strbld<<Cmds[3].arg(i).arg(i).toStdString();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i].toStdString();
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateTLevel()
{
    QFile f(":/dbtemplate/cdbtemplate/tleveltemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table t_level and index
    strbld<<Cmds[0].toStdString()
            <<Cmds[1].toStdString();
    if(!_bPartitionTable)
    {
        return strbld.str();
    }
    std::stringstream strbld1;
    int startnum;
    int endnum;
    //template for partition table and index
    for(int i=1;i<=_tablecnt1;++i)
    {
        startnum=(i-1)*_idcnt1+1;
        if(i==_tablecnt1)
        {
            endnum=_maxid;
        }
        else
        {
            endnum=i*_idcnt1;
        }
        if(i==1)
        {
            strbld1<<Cmds[4].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        else
        {
            strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        strbld<<Cmds[2].arg(i).arg(i).arg(i).arg(startnum).arg(endnum).toStdString();
        strbld<<Cmds[3].arg(i).arg(i).toStdString();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i].toStdString();
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateNodeinfo()
{
    QFile f(":/dbtemplate/cdbtemplate/nodinfotemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table nod_info and index
    strbld<<Cmds[0].toStdString()
            <<Cmds[1].toStdString();
    if(!_bPartitionTable || _tablecnt2==1)
    {
        return strbld.str();
    }
    std::stringstream strbld1;
    int startnum;
    int endnum;
    //template for partition table and index
    for(int i=1;i<=_tablecnt2;++i)
    {
        startnum=(i-1)*_idcnt2+1;
        if(i==_tablecnt2)
        {
            endnum=_maxid;
        }
        else
        {
            endnum=i*_idcnt2;
        }
        if(i==1)
        {
            strbld1<<Cmds[4].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        else
        {
            strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        strbld<<Cmds[2].arg(i).arg(i).arg(i).arg(startnum).arg(endnum).toStdString();
        strbld<<Cmds[3].arg(i).arg(i).toStdString();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i].toStdString();
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateObsNode()
{
    QFile f(":/dbtemplate/cdbtemplate/obsnodetemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table obs_node and index
    strbld<<Cmds[0].toStdString()
            <<Cmds[1].toStdString();
    if(!_bPartitionTable || _tablecnt2==1)
    {
        return strbld.str();
    }
    std::stringstream strbld1;
    int startnum;
    int endnum;
    //template for partition table and index
    for(int i=1;i<=_tablecnt2;++i)
    {
        startnum=(i-1)*_idcnt2+1;
        if(i==_tablecnt2)
        {
            endnum=_maxid;
        }
        else
        {
            endnum=i*_idcnt2;
        }
        if(i==1)
        {
            strbld1<<Cmds[4].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        else
        {
            strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        strbld<<Cmds[2].arg(i).arg(i).arg(i).arg(startnum).arg(endnum).toStdString();
        strbld<<Cmds[3].arg(i).arg(i).toStdString();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i].toStdString();
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateSolute()
{
    QFile f(":/dbtemplate/cdbtemplate/solutetemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table solute and index
    for(int i=1;i<=10;++i)
    {
        strbld<<Cmds[0].arg(i).arg(i).toStdString()
                <<Cmds[1].arg(i).arg(i).toStdString();
        if(!_bPartitionTable)
        {
            continue;
        }
        std::stringstream strbld1;
        int startnum;
        int endnum;
        //template for partition table and index
        for(int j=1;j<=_tablecnt1;++j)
        {
            startnum=(j-1)*_idcnt1+1;
            if(j==_tablecnt1)
            {
                endnum=_maxid;
            }
            else
            {
                endnum=j*_idcnt1;
            }
            if(j==1)
            {
                strbld1<<Cmds[4].arg(i).arg(startnum).arg(endnum).arg(i).arg(j).toStdString();
            }
            else
            {
                strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).arg(j).toStdString();
            }
            strbld<<Cmds[2].arg(i).arg(j).arg(i).arg(j).arg(i).arg(j)
                    .arg(startnum).arg(endnum).arg(i).toStdString();
            strbld<<Cmds[3].arg(i).arg(j).arg(i).arg(j).toStdString();
        }
        for(unsigned int i=6;i<Cmds.size()-1;++i)
        {
            strbld1<<Cmds[i].toStdString();
        }
        strbld1<<Cmds.back().arg(i).arg(i).arg(i).toStdString();
        strbld<<strbld1.str();
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateProfile()
{
    QFile f(":/dbtemplate/cdbtemplate/profiletemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table profile and index
    strbld<<Cmds[0].toStdString()
            <<Cmds[1].toStdString();
    if(!_bPartitionTable || _tablecnt2==1)
    {
        return strbld.str();
    }
    std::stringstream strbld1;
    int startnum;
    int endnum;
    //template for partition table and index
    for(int i=1;i<=_tablecnt2;++i)
    {
        startnum=(i-1)*_idcnt2+1;
        if(i==_tablecnt2)
        {
            endnum=_maxid;
        }
        else
        {
            endnum=i*_idcnt2;
        }
        if(i==1)
        {
            strbld1<<Cmds[4].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        else
        {
            strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        strbld<<Cmds[2].arg(i).arg(i).arg(i).arg(startnum).arg(endnum).toStdString();
        strbld<<Cmds[3].arg(i).arg(i).toStdString();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i].toStdString();
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateAtmosph()
{
    QFile f(":/dbtemplate/cdbtemplate/atmosphtemplate");
    std::vector<QString> Cmds;
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString cmd;
        QString line;
        QTextStream stm(&f);
        while (!stm.atEnd())
        {
            line=stm.readLine();
            cmd.append(line).append("\n");
            if(line.back()==';')
            {
                Cmds.push_back(cmd);
                cmd.clear();
            }
        }
    }
    else
    {
        return "";
    }
    std::stringstream strbld;
    //create table atmosph and index
    strbld<<Cmds[0].toStdString()
            <<Cmds[1].toStdString();
    if(!_bPartitionTable)
    {
        return strbld.str();
    }
    std::stringstream strbld1;
    int startnum;
    int endnum;
    //template for partition table and index
    for(int i=1;i<=_tablecnt1;++i)
    {
        startnum=(i-1)*_idcnt1+1;
        if(i==_tablecnt1)
        {
            endnum=_maxid;
        }
        else
        {
            endnum=i*_idcnt1;
        }
        if(i==1)
        {
            strbld1<<Cmds[4].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        else
        {
            strbld1<<Cmds[5].arg(startnum).arg(endnum).arg(i).toStdString();
        }
        strbld<<Cmds[2].arg(i).arg(i).arg(i).arg(startnum).arg(endnum).toStdString();
        strbld<<Cmds[3].arg(i).arg(i).toStdString();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i].toStdString();
    }
    strbld<<strbld1.str();
    return strbld.str();
}
