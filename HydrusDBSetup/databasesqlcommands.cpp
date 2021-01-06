
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

#include <sstream>
#include <iostream>
#include <limits>
#include "Stringhelper.h"
#include "databasesqlcommands.h"
#include "aleveltemplate.h"
#include "tleveltemplate.h"
#include "functiontemplate.h"
#include "dbtemplate.h"
#include "atmosphtemplate.h"
#include "nodinfotemplate.h"
#include "obsnodetemplate.h"
#include "solutetemplate.h"
#include "profiletemplate.h"
#include "selectortemplate.h"


DataBaseSQLCommands::DataBaseSQLCommands()
    :_bPartitionTable(false),_tablecnt1(0),_idcnt1(0),_tablecnt2(0),_idcnt2(0),_maxid(0)
{
}

DataBaseSQLCommands::DataBaseSQLCommands(const int tablecount, const int gidcount)
    :_tablecnt1(tablecount),_idcnt1(gidcount),_idcnt2(1000),_maxid(std::numeric_limits<int>::max()-1)
{
    _bPartitionTable=tablecount>1;
    _tablecnt2=int(tablecount*gidcount/1000.0)+1;
}

std::string DataBaseSQLCommands::GetCreateDbSqlCommand(const std::string& dbname)
{
    std::string content(reinterpret_cast<char*>(&dbtemplate[0]),dbtemplate_len);
    std::stringstream instream(content);
    std::string line;
    std::string cmd;
    while(std::getline(instream,line))
    {
        if(line.back()==';')
        {
            cmd.append(line);
            break;
        }
        cmd.append(line).append("\n");
    }
    return Stringhelper(cmd).arg(dbname).str();
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
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&selectortemplate[0]),selectortemplate_len);
    std::stringstream instream(content);

    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    for(unsigned int i=0;i<Cmds.size();++i)
    {
        strbld<<Cmds[i];
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateFunction()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&functiontemplate[0]),functiontemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    for(unsigned int i=0;i<Cmds.size();++i)
    {
        strbld<<Cmds[i];
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateALevel()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&aleveltemplate[0]),aleveltemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table a_level and index
    strbld<<Cmds[0]<<Cmds[1];
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
            strbld1<<Stringhelper(Cmds[4]).arg(startnum).arg(endnum).arg(i).str();
        }
        else
        {
            strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).str();
        }
        strbld<<Stringhelper(Cmds[2]).arg(i).arg(i).arg(i).arg(startnum).arg(endnum).str();
        strbld<<Stringhelper(Cmds[3]).arg(i).arg(i).str();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i];
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateTLevel()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&tleveltemplate[0]),tleveltemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table t_level and index
    strbld<<Cmds[0]<<Cmds[1];
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
            strbld1<<Stringhelper(Cmds[4]).arg(startnum).arg(endnum).arg(i).str();
        }
        else
        {
            strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).str();
        }
        strbld<<Stringhelper(Cmds[2]).arg(i).arg(i).arg(i).arg(startnum).arg(endnum).str();
        strbld<<Stringhelper(Cmds[3]).arg(i).arg(i).str();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i];
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateNodeinfo()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&nodinfotemplate[0]),nodinfotemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table nod_info and index
    strbld<<Cmds[0]<<Cmds[1];
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
            strbld1<<Stringhelper(Cmds[4]).arg(startnum).arg(endnum).arg(i).str();
        }
        else
        {
            strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).str();
        }
        strbld<<Stringhelper(Cmds[2]).arg(i).arg(i).arg(i).arg(startnum).arg(endnum).str();
        strbld<<Stringhelper(Cmds[3]).arg(i).arg(i).str();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i];
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateObsNode()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&obsnodetemplate[0]),obsnodetemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table obs_node and index
    strbld<<Cmds[0]<<Cmds[1];
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
            strbld1<<Stringhelper(Cmds[4]).arg(startnum).arg(endnum).arg(i).str();
        }
        else
        {
            strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).str();
        }
        strbld<<Stringhelper(Cmds[2]).arg(i).arg(i).arg(i).arg(startnum).arg(endnum).str();
        strbld<<Stringhelper(Cmds[3]).arg(i).arg(i).str();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i];
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateSolute()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&solutetemplate[0]),solutetemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table solute and index
    for(int i=1;i<=10;++i)
    {
        strbld<<Stringhelper(Cmds[0]).arg(i).arg(i).str()
                <<Stringhelper(Cmds[1]).arg(i).arg(i).str();
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
                strbld1<<Stringhelper(Cmds[4]).arg(i).arg(startnum).arg(endnum).arg(i).arg(j).str();
            }
            else
            {
                strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).arg(j).str();
            }
            strbld<<Stringhelper(Cmds[2]).arg(i).arg(j).arg(i).arg(j).arg(i).arg(j)
                    .arg(startnum).arg(endnum).arg(i).str();
            strbld<<Stringhelper(Cmds[3]).arg(i).arg(j).arg(i).arg(j).str();
        }
        for(unsigned int i=6;i<Cmds.size()-1;++i)
        {
            strbld1<<Cmds[i];
        }
        strbld1<<Stringhelper(Cmds.back()).arg(i).arg(i).arg(i).str();
        strbld<<strbld1.str();
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateProfile()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&profiletemplate[0]),profiletemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table profile and index
    strbld<<Cmds[0]<<Cmds[1];
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
            strbld1<<Stringhelper(Cmds[4]).arg(startnum).arg(endnum).arg(i).str();
        }
        else
        {
            strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).str();
        }
        strbld<<Stringhelper(Cmds[2]).arg(i).arg(i).arg(i).arg(startnum).arg(endnum).str();
        strbld<<Stringhelper(Cmds[3]).arg(i).arg(i).str();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i];
    }
    strbld<<strbld1.str();
    return strbld.str();
}

std::string DataBaseSQLCommands::CreateAtmosph()
{
    std::string cmd;
    std::string line;
    std::vector<std::string> Cmds;
    std::string content(reinterpret_cast<char*>(&atmosphtemplate[0]),atmosphtemplate_len);
    std::stringstream instream(content);
    while (std::getline(instream,line))
    {
        cmd.append(line).append("\n");
        if(line.back()==';')
        {
            Cmds.push_back(cmd);
            cmd.clear();
        }
    }
    std::stringstream strbld;
    //create table atmosph and index
    strbld<<Cmds[0]<<Cmds[1];
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
            strbld1<<Stringhelper(Cmds[4]).arg(startnum).arg(endnum).arg(i).str();
        }
        else
        {
            strbld1<<Stringhelper(Cmds[5]).arg(startnum).arg(endnum).arg(i).str();
        }
        strbld<<Stringhelper(Cmds[2]).arg(i).arg(i).arg(i).arg(startnum).arg(endnum).str();
        strbld<<Stringhelper(Cmds[3]).arg(i).arg(i).str();
    }
    for(unsigned int i=6;i<Cmds.size();++i)
    {
        strbld1<<Cmds[i];
    }
    strbld<<strbld1.str();
    return strbld.str();
}
