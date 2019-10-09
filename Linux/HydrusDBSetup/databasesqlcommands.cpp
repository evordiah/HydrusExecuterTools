
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

#include "databasesqlcommands.h"
#include "sstream"
#include <iostream>
#include <limits>

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
    std::string v=_sqlcommands[0];
    auto pos=v.find("[DBNAME]");
    return v.replace(pos,8,dbname);
}

std::string DataBaseSQLCommands::GetCreateTablesSqlCommand()
{
    std::stringstream strbld;
    for(unsigned int i=6;i<_sqlcommands.size();++i)
    {
        if(_sqlcommands[i]=="[PARTITION TABLE SEGMENT]\n")
        {
            if( _bPartitionTable)
            {
                strbld<<CreatePartitionTablesForALevel()<<std::endl;
                strbld<<CreatePartitionTablesForAtmosph()<<std::endl;
                strbld<<CreatePartitionTablesForNodeinfo()<<std::endl;
                strbld<<CreatePartitionTablesForProfile()<<std::endl;
                strbld<<CreatePartitionTablesForTLevel()<<std::endl;
            }
        }
        else
        {
           strbld<<_sqlcommands[i]<<std::endl;
        }
    }
    return strbld.str();
}

std::string DataBaseSQLCommands::CreatePartitionTablesForALevel()
{
    std::stringstream strbld;
    std::stringstream strbld1;
    std::string strindex;
    std::string strsnum;
    std::string strenum;
    std::string v;

    strbld<<"CREATE OR REPLACE FUNCTION public.a_level_insert_trig() \n"
            "RETURNS trigger AS \n"
            "$BODY$ \n"
            "BEGIN \n";
    for(int i=1;i<=_tablecnt1;++i)
    {
        strindex=std::to_string(i);
        strsnum=std::to_string((i-1)*_idcnt1+1);
        if(i==_tablecnt1)
        {
            strenum=std::to_string(_maxid);
        }
        else
        {
            strenum=std::to_string(i*_idcnt1);
        }
        if(i==1)
        {
            strbld<<"if (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                  <<"    insert into a_level_"<<strindex<<" values(new.*);\n";
        }
        else
        {
            strbld<<"elsif (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                 <<"    insert into a_level_"<<strindex<<" values(new.*);\n";
        }
        v=_sqlcommands[1];
        auto pos=v.find("[S]");
        v.replace(pos,3,strsnum);
        pos=v.find("[E]");
        v.replace(pos,3,strenum);
        pos=v.find("[NUM]");
        while(pos!=std::string::npos)
        {
            v.replace(pos,5,strindex);
            pos=v.find("[NUM]",pos+5);
        }
        strbld1<<v;
    }
    strbld<<"else\n"
            "   raise exception 'gid over the range';\n"
            "end if;\n"
            "return null;\n"
            "end;\n"
            "$BODY$\n"
            "  LANGUAGE plpgsql;\n"
            //"ALTER FUNCTION public.a_level_insert_trig()\n"
            //"  OWNER TO postgres;\n"
            "CREATE TRIGGER a_level_insert\n"
            "BEFORE INSERT\n"
            "ON public.a_level\n"
            "FOR EACH ROW\n"
            "EXECUTE PROCEDURE public.a_level_insert_trig();\n";
    strbld1<<strbld.str();
    return strbld1.str();
}

std::string DataBaseSQLCommands::CreatePartitionTablesForTLevel()
{
    std::stringstream strbld;
    std::stringstream strbld1;
    std::string strindex;
    std::string strsnum;
    std::string strenum;
    std::string v;

    strbld<<"CREATE OR REPLACE FUNCTION public.t_level_insert_trig() \n"
            "RETURNS trigger AS \n"
            "$BODY$ \n"
            "BEGIN \n";
    for(int i=1;i<=_tablecnt1;++i)
    {
        strindex=std::to_string(i);
        strsnum=std::to_string((i-1)*_idcnt1+1);
        if(i==_tablecnt1)
        {
            strenum=std::to_string(_maxid);
        }
        else
        {
            strenum=std::to_string(i*_idcnt1);
        }
        if(i==1)
        {
            strbld<<"if (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                  <<"    insert into t_level_"<<strindex<<" values(new.*);\n";
        }
        else
        {
            strbld<<"elsif (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                 <<"    insert into t_level_"<<strindex<<" values(new.*);\n";
        }
        v=_sqlcommands[2];
        auto pos=v.find("[S]");
        v.replace(pos,3,strsnum);
        pos=v.find("[E]");
        v.replace(pos,3,strenum);
        pos=v.find("[NUM]");
        while(pos!=std::string::npos)
        {
            v.replace(pos,5,strindex);
            pos=v.find("[NUM]",pos+5);
        }
        strbld1<<v;
    }
    strbld<<"else\n"
            "   raise exception 'gid over the range';\n"
            "end if;\n"
            "return null;\n"
            "end;\n"
            "$BODY$\n"
            "  LANGUAGE plpgsql;\n"
            //"ALTER FUNCTION public.t_level_insert_trig()\n"
            //"  OWNER TO postgres;\n"
            "CREATE TRIGGER t_level_insert\n"
            "BEFORE INSERT\n"
            "ON public.t_level\n"
            "FOR EACH ROW\n"
            "EXECUTE PROCEDURE public.t_level_insert_trig();\n";

    strbld1<<strbld.str();
    return strbld1.str();
}

std::string DataBaseSQLCommands::CreatePartitionTablesForNodeinfo()
{
    std::stringstream strbld;
    std::stringstream strbld1;
    std::string strindex;
    std::string strsnum;
    std::string strenum;
    std::string v;

    strbld<<"CREATE OR REPLACE FUNCTION public.nod_info_insert_trig() \n"
            "RETURNS trigger AS \n"
            "$BODY$ \n"
            "BEGIN \n";
    for(int i=1;i<=_tablecnt2;++i)
    {
        strindex=std::to_string(i);
        strsnum=std::to_string((i-1)*_idcnt2+1);
        if(i==_tablecnt2)
        {
            strenum=std::to_string(_maxid);
        }
        else
        {
            strenum=std::to_string(i*_idcnt2);
        }
        if(i==1)
        {
            strbld<<"if (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                  <<"    insert into nod_info_"<<strindex<<" values(new.*);\n";
        }
        else
        {
            strbld<<"elsif (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                 <<"    insert into nod_info_"<<strindex<<" values(new.*);\n";
        }
        v=_sqlcommands[3];
        auto pos=v.find("[S]");
        v.replace(pos,3,strsnum);
        pos=v.find("[E]");
        v.replace(pos,3,strenum);
        pos=v.find("[NUM]");
        while(pos!=std::string::npos)
        {
            v.replace(pos,5,strindex);
            pos=v.find("[NUM]",pos+5);
        }
        strbld1<<v;
    }
    strbld<<"else\n"
            "   raise exception 'gid over the range';\n"
            "end if;\n"
            "return null;\n"
            "end;\n"
            "$BODY$\n"
            "  LANGUAGE plpgsql;\n"
            //"ALTER FUNCTION public.nod_info_insert_trig()\n"
            //"  OWNER TO postgres;\n"
            "CREATE TRIGGER nod_info_insert\n"
            "BEFORE INSERT\n"
            "ON public.nod_info\n"
            "FOR EACH ROW\n"
            "EXECUTE PROCEDURE public.nod_info_insert_trig();\n";

    strbld1<<strbld.str();
    return strbld1.str();
}

std::string DataBaseSQLCommands::CreatePartitionTablesForProfile()
{
    std::stringstream strbld;
    std::stringstream strbld1;
    std::string strindex;
    std::string strsnum;
    std::string strenum;
    std::string v;

    strbld<<"CREATE OR REPLACE FUNCTION public.profile_insert_trig() \n"
            "RETURNS trigger AS \n"
            "$BODY$ \n"
            "BEGIN \n";
    for(int i=1;i<=_tablecnt2;++i)
    {
        strindex=std::to_string(i);
        strsnum=std::to_string((i-1)*_idcnt2+1);
        if(i==_tablecnt2)
        {
            strenum=std::to_string(_maxid);
        }
        else
        {
            strenum=std::to_string(i*_idcnt2);
        }
        if(i==1)
        {
            strbld<<"if (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                  <<"    insert into profile_"<<strindex<<" values(new.*);\n";
        }
        else
        {
            strbld<<"elsif (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                 <<"    insert into profile_"<<strindex<<" values(new.*);\n";
        }
        v=_sqlcommands[4];
        auto pos=v.find("[S]");
        v.replace(pos,3,strsnum);
        pos=v.find("[E]");
        v.replace(pos,3,strenum);
        pos=v.find("[NUM]");
        while(pos!=std::string::npos)
        {
            v.replace(pos,5,strindex);
            pos=v.find("[NUM]",pos+5);
        }
        strbld1<<v;
    }
    strbld<<"else\n"
            "   raise exception 'gid over the range';\n"
            "end if;\n"
            "return null;\n"
            "end;\n"
            "$BODY$\n"
            "  LANGUAGE plpgsql;\n"
            //"ALTER FUNCTION public.profile_insert_trig()\n"
            //"  OWNER TO postgres;\n"
            "CREATE TRIGGER profile_insert\n"
            "BEFORE INSERT\n"
            "ON public.profile\n"
            "FOR EACH ROW\n"
            "EXECUTE PROCEDURE public.profile_insert_trig();\n";
    strbld1<<strbld.str();
    return strbld1.str();
}

std::string DataBaseSQLCommands::CreatePartitionTablesForAtmosph()
{
    std::stringstream strbld;
    std::stringstream strbld1;
    std::string strindex;
    std::string strsnum;
    std::string strenum;
    std::string v;

    strbld<<"CREATE OR REPLACE FUNCTION public.atmosph_insert_trig() \n"
            "RETURNS trigger AS \n"
            "$BODY$ \n"
            "BEGIN \n";
    for(int i=1;i<=_tablecnt1;++i)
    {
        strindex=std::to_string(i);
        strsnum=std::to_string((i-1)*_idcnt1+1);
        if(i==_tablecnt1)
        {
            strenum=std::to_string(_maxid);
        }
        else
        {
            strenum=std::to_string(i*_idcnt1);
        }
        if(i==1)
        {
            strbld<<"if (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                  <<"    insert into atmosph_"<<strindex<<" values(new.*);\n";
        }
        else
        {
            strbld<<"elsif (new.gid between "<<strsnum<<" and "<<strenum<<") then \n"
                 <<"    insert into atmosph_"<<strindex<<" values(new.*);\n";
        }
        v=_sqlcommands[5];
        auto pos=v.find("[S]");
        v.replace(pos,3,strsnum);
        pos=v.find("[E]");
        v.replace(pos,3,strenum);
        pos=v.find("[NUM]");
        while(pos!=std::string::npos)
        {
            v.replace(pos,5,strindex);
            pos=v.find("[NUM]",pos+5);
        }
        strbld1<<v;
    }
    strbld<<"else\n"
            "   raise exception 'gid over the range';\n"
            "end if;\n"
            "return null;\n"
            "end;\n"
            "$BODY$\n"
            "  LANGUAGE plpgsql;\n"
            //"ALTER FUNCTION public.atmosph_insert_trig()\n"
            //"  OWNER TO postgres;\n"
            "CREATE TRIGGER atmosph_insert\n"
            "BEFORE INSERT\n"
            "ON public.atmosph\n"
            "FOR EACH ROW\n"
            "EXECUTE PROCEDURE public.atmosph_insert_trig();\n";
    strbld1<<strbld.str();
    return strbld1.str();
}
