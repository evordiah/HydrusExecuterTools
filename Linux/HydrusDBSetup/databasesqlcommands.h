
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

#ifndef DATABASESQLCOMMANDS_H
#define DATABASESQLCOMMANDS_H
#include <vector>
#include <string>

class DataBaseSQLCommands
{
public:
    DataBaseSQLCommands();
    DataBaseSQLCommands(const int tablecount,const int gidcount);
    std::string GetCreateDbSqlCommand(const std::string &dbname);
    std::string GetCreateTablesSqlCommand();
protected:
    std::string CreatePartitionTablesForALevel();
    std::string CreatePartitionTablesForTLevel();
    std::string CreatePartitionTablesForNodeinfo();
    std::string CreatePartitionTablesForProfile();
    std::string CreatePartitionTablesForAtmosph();
private:
    bool _bPartitionTable;
    int _tablecnt1; //for table such as a_level,t_level,atmosph
    int _idcnt1;    //for table such as a_level,t_level,atmosph
    int _tablecnt2; //for table such as nod_info,profile
    const int _idcnt2; //for table such as nod_info,profile
    const int _maxid;
    std::vector<std::string> _sqlcommands={
        "CREATE DATABASE [DBNAME] TEMPLATE = template_postgis;\n",
        "CREATE TABLE public.a_level_[NUM](\n"
        "  CONSTRAINT a_level_[NUM]_pk PRIMARY KEY (gid, tm),\n"
        "  CONSTRAINT a_level_[NUM]_gid_check CHECK (gid >= [S] AND gid <= [E]))\n"
        "INHERITS (public.a_level) WITH (OIDS=FALSE);\n"
        //"--ALTER TABLE public.a_level_[NUM] OWNER TO postgres;\n"
        "CREATE INDEX a_level_[NUM]_gid_idx ON public.a_level_[NUM] USING btree(gid);\n",
        "CREATE TABLE public.t_level_[NUM](\n"
        "  CONSTRAINT t_level_[NUM]_pk PRIMARY KEY (gid, tm),\n"
        "  CONSTRAINT t_level_[NUM]_gid_check CHECK (gid >= [S] AND gid <= [E]))\n"
        "INHERITS (public.t_level) WITH (OIDS=FALSE);\n"
        //"--ALTER TABLE public.t_level_[NUM] OWNER TO postgres;\n"
        "CREATE INDEX t_level_[NUM]_gid_idx ON public.t_level_[NUM] USING btree(gid);\n",
        "CREATE TABLE public.nod_info_[NUM](\n"
        "  CONSTRAINT nod_info_[NUM]_pk PRIMARY KEY (gid, tm, node),\n"
        "  CONSTRAINT nod_info_[NUM]_gid_check CHECK (gid >= [S] AND gid <= [E]))\n"
        "INHERITS (public.nod_info) WITH (OIDS=FALSE);\n"
        //"--ALTER TABLE public.nod_info_[NUM] OWNER TO postgres;\n"
        "CREATE INDEX nod_info_[NUM]_gid_idx ON public.nod_info_[NUM] USING btree(gid);\n",
        "CREATE TABLE public.profile_[NUM](\n"
        "  CONSTRAINT profile_[NUM]_pk PRIMARY KEY (gid, node),\n"
        "  CONSTRAINT profile_[NUM]_gid_check CHECK (gid >= [S] AND gid <= [E]))\n"
        "INHERITS (public.profile) WITH (OIDS=FALSE);\n"
        //"--ALTER TABLE public.profile_[NUM] OWNER TO postgres;\n"
        "CREATE INDEX profile_[NUM]_gid_idx ON public.profile_[NUM] USING btree(gid);\n",
        "CREATE TABLE public.atmosph_[NUM](\n"
        "  CONSTRAINT atmosph_[NUM]_pk PRIMARY KEY (gid, tm),\n"
        "  CONSTRAINT atmosph_[NUM]_gid_check CHECK (gid >= [S] AND gid <= [E]))\n"
        "INHERITS (public.atmosph) WITH (OIDS=FALSE);\n"
        //"--ALTER TABLE public.atmosph_[NUM] OWNER TO postgres;\n"
        "CREATE INDEX atmosph_[NUM]_gid_idx ON public.atmosph_[NUM] USING btree(gid);\n",
        "SET statement_timeout = 0;\n",
        "SET lock_timeout = 0;\n",
        "SET client_encoding = 'UTF8';\n",
        "SET standard_conforming_strings = on;\n",
        "SET check_function_bodies = false;\n",
        "SET client_min_messages = warning;\n",
        "SET row_security = off;\n",
        "CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;\n",
        "COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';\n",
        "SET search_path = public, pg_catalog;\n",
        "CREATE FUNCTION clearerrlog() RETURNS void\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "delete from errlog ;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.clearerrlog() OWNER TO postgres;\n",
        "CREATE FUNCTION clearerrlogbyid(id integer) RETURNS void\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "delete from errlog where gid=id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.clearerrlogbyid(id integer) OWNER TO postgres;\n",
        "CREATE FUNCTION clearhydrusresultbyid(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS void\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion=on;\n"
        "delete from a_level where gid=id;\n"
        "delete from t_level where gid=id;\n"
        "delete from nod_info where gid=id;\n"
        "delete from waterbalance where gid=id;\n"
        "delete from obsnode where gid=id;\n"
        "delete from errlog where gid=id;\n"
        "update selector set status='todo' where gid=id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.clearhydrusresultbyid(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION deletehydrusinputparambyid(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS void\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion=on;\n"
        "delete from selector where gid=id;\n"
        "delete from profile where gid=id;\n"
        "delete from profilestatistics where gid=id;\n"
        "delete from atmosph where gid=id;\n"
        "delete from atmosphstatistics where gid=id;\n"
        "delete from errlog where gid=id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.deletehydrusinputparambyid(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getalevelcount(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS integer\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "cnt integer;\n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "select count(*) into cnt from a_level where gid=id;\n"
        "return cnt;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getalevelcount(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getaleveldata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(tm real, sr_top real, sr_root real, sv_top real, sv_root real, sv_bot real, htop real, hroot real, hbot real, a_level integer)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query SELECT a.tm ,a.sr_top,a.sr_root,a.sv_top, a.sv_root, a.sv_bot,  a.htop, a.hroot, a.hbot,a.a_level \n"
        "                FROM a_level a where a.gid=id order by a.tm;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getaleveldata(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getatmosphdata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0, parameter2 integer DEFAULT 0) RETURNS TABLE(tm real, prec_cm real, rsoil_cm real, rroot_cm real, hcrita real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query select a.tm, a.prec_cm ,a.rsoil_cm ,a.rroot_cm ,a.hcrita from atmosph a where a.gid=id order by tm;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getatmosphdata(id integer, parameter0 integer, parameter1 integer, parameter2 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getatmosphinfo(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(cnt integer, hcrits real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "cnt integer;\n"
        "begin\n"
        "select count(*) into cnt from atmosphstatistics a where a.gid = id;\n"
        "if cnt=0 then\n"
        "   insert into atmosphstatistics(gid,recordcnt) \n"
        "   select a.gid, count(*) from atmosph a where a.gid=id group by a.gid;\n"
        "end if;\n"
        "return query select a.recordcnt,a.hcrits from atmosphstatistics a where a.gid= id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getatmosphinfo(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getbeta(x_coord real, maxrootuptakedepth real, bremovetop boolean DEFAULT false, toplength real DEFAULT 4.0) RETURNS real\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "result real;\n"
        "begin\n"
        "if bremovetop then\n"
        "   if x_coord > -toplength then\n"
        "      return 0;\n"
        "   end if;\n"
        "end if;\n"
        "if x_coord>(-0.2)*maxrootuptakedepth then\n"
        "   result:=1.667/maxrootuptakedepth;\n"
        "elsif x_coord<(-1)* maxrootuptakedepth then\n"
        "   result:=0;\n"
        "else \n"
        "   result:=(2.0833/maxrootuptakedepth)*(1+x_coord/(maxrootuptakedepth));\n"
        "end if;\n"
        "return result;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getbeta(x_coord real, maxrootuptakedepth real, bremovetop boolean, toplength real) OWNER TO postgres;\n",
        "CREATE FUNCTION getdesinfo(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(tunit text, lunit text, dtandtm text, caltm double precision)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "return query select cast(s.tunit as text), cast(s.lunit as text),cast(s.dtandtm as text),cast(s.caltm as double precision) from selector s where s.gid= id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getdesinfo(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getnodinfo(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(tm real, cnt smallint)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query SELECT n.tm,cast (count(*) as smallint) as cnt from nod_info n where n.gid=id\n"
        "          group by n.tm order by tm; \n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getnodinfo(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getnodinfodata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(node smallint, depth real, head real, moistrue real, k real, c real, darcian_velocity real, sink real, kappa real, vdivkstop real, temp real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query SELECT n.node, n.depth, n.head, n.moistrue, n.k, n.c, n.darcian_velocity, n.sink, n.kappa, n.vdivkstop, n.temp \n"
        "             FROM nod_info n where n.gid=id order by n.tm,n.node;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getnodinfodata(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getobsnodedata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(tm real, node smallint, h real, theta real, flux real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion=on;\n"
        "return query select o.tm,o.node,o.h,o.theta,o.flux from obsnode o where o.gid=id order by o.tm,o.node;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getobsnodedata(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getobsnodeinfo(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS text\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "cnt1 integer;\n"
        "cnt2 integer;\n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "select count( distinct tm ) into cnt1 from obsnode where gid=id;\n"
        "select count(distinct node) into cnt2 from obsnode where gid=id;\n"
        "return cast(cnt1 as text) || '  ' || cast(cnt2 as text);\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getobsnodeinfo(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getprofiledata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(x_coord real, matnum smallint, laynum smallint, beta real, h real, ah real, ak real, ath real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion=on;\n"
        "return query select p.x_coord,p.matnum,p.laynum,p.beta,p.h,p.ah,p.ak,p.ath from profile p where p.gid=id order by p.node;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getprofiledata(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getprofileinfo(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(depth real, nodecnt smallint, observercnt smallint, observerid smallint[])\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "cnt integer;\n"
        "begin\n"
        "select count(*) into cnt from profilestatistics p where p.gid=id;\n"
        "if cnt=0 then\n"
        "   insert into profilestatistics(gid,depth,nodecnt) \n"
        "   select gid,abs(min(x_coord)) as depth,max(node) as nodecnt  from profile p where p.gid =id  group by p.gid;\n"
        "end if;\n" 
        "return query select p.depth,p.nodecnt,p.observercnt,p.observerid from profilestatistics p where p.gid=id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getprofileinfo(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getselectordata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0, parameter2 integer DEFAULT 0) RETURNS TABLE(maxtime real, nmat smallint, nlay smallint, printinterval real, p0 real, p2h real, p2l real, p3 real, printtimes smallint, model smallint, rootgrowthcnt smallint, r2h real, r2l real, omegac real, matdata real[], printtimedata real[], rootdate real[], rootlength real[], lunit character varying, tunit character varying, lroot smallint, lsink smallint, lwlayer smallint, linitw smallint, maxit smallint, itmin smallint, itmax smallint, tolth real, tolh real, ha real, hb real, dt real, dtmin real, dtmax real, dmul real, dmul2 real, inittime real, poptm real[])\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "return query SELECT s.maxtime, s.nmat, s.nlay, s.printinterval, s.p0, s.p2h, s.p2l,\n"
        "             s.p3, s.printtimes, s.model, s.rootgrowthcnt, s.r2h, s.r2l, s.omegac, s.matdata,\n"
        "             s.printtimedata, s.rootdate, s.rootlength, s.lunit,s.tunit,\n"
        "             s.lroot,s.lsink,s.lwlayer,s.linitw, s.maxit,s.itmin,s.itmax,\n"
        "             s.tolth,s.tolh,s.ha,s.hb,s.dt,s.dtmin,s.dtmax,s.dmul,s.dmul2,s.inittime,s.poptm \n"
        "             FROM selector s where gid=id;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getselectordata(id integer, parameter0 integer, parameter1 integer, parameter2 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION gettlevelcount(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS integer\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "cnt integer;\n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "select count(*) into cnt from t_level where gid=id;\n"
        "return cnt;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.gettlevelcount(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION gettleveldata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(tm real, rtop real, rroot real, vtop real, vroot real, vbot real, sr_top real, sr_root real, sv_top real, sv_root real, sv_bot real, htop real, hroot real, hbot real, runoff real, s_runoff real, vol real, s_infil real, s_evap real, t_level integer, cum_wtrans real, snowlayer real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query SELECT t.tm ,t.rtop,t.rroot,t.vtop,t.vroot, t.vbot,t.sr_top, t.sr_root, t.sv_top, t.sv_root, t.sv_bot,\n"
        "            t.htop, t.hroot, t.hbot, t.runoff,t.s_runoff,t.vol, t.s_infil, t.s_evap,\n"
        "            t.t_level, t.cum_wtrans, t.snowlayer FROM t_level t where t.gid=id order by t.tm;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.gettleveldata(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getwaterbalancedata(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(length real, w_volume real, in_flow real, h_mean real, top_flux real, bot_flux real, watbalt real, watbalr real)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query SELECT w.length, w.w_volume, w.in_flow, w.h_mean, w.top_flux,\n"
        "            w.bot_flux, w.watbalt, w.watbalr FROM waterbalance w WHERE w.GID=id\n"
        "            order by w.tm,w.subregion; \n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getwaterbalancedata(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION getwaterbalanceinfo(id integer, parameter0 integer DEFAULT 0, parameter1 integer DEFAULT 0) RETURNS TABLE(tm real, subregioncnt smallint)\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "set constraint_exclusion = on;\n"
        "return query select w.tm, cast(count(*) as smallint) as subregioncnt from waterbalance w where w.gid=id group by w.tm order by w.tm; \n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.getwaterbalanceinfo(id integer, parameter0 integer, parameter1 integer) OWNER TO postgres;\n",
        "CREATE FUNCTION removehydruscasebyid(id integer) RETURNS void\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "begin\n"
        "perform deletehydrusinputparambyid(id);\n"
        "perform clearhydrusresultbyid(id);\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.removehydruscasebyid(id integer) OWNER TO postgres;\n",
        "CREATE FUNCTION updateselectorstatus(id integer, bsucc boolean DEFAULT false, dttm character varying DEFAULT ''::character varying, ctm numeric DEFAULT 0) RETURNS void\n"
        "    LANGUAGE plpgsql\n"
        "    AS $$\n"
        "declare \n"
        "ss varchar(10):='done';\n"
        "begin\n"
        "if bSucc=false then\n"
        "   ss:='todo';\n"
        "   dttm:='';\n"
        "   ctm:=0;\n"
        "end if;\n"
        "update selector set status=ss,dtandtm=dttm, caltm=ctm where gid=id;\n"
        "if bSucc=true then\n"
        "   perform clearerrlogbyid(id);\n"
        "end if;\n"
        "end;\n"
        "$$;\n",
        //"--ALTER FUNCTION public.updateselectorstatus(id integer, bsucc boolean, dttm character varying, ctm numeric) OWNER TO postgres;\n",
        "SET default_tablespace = '';\n",
        "SET default_with_oids = false;\n",
        "CREATE TABLE a_level (\n"
        "    gid integer NOT NULL,\n"
        "    tm real NOT NULL,\n"
        "    sv_top real,\n"
        "    sv_root real,\n"
        "    sv_bot real,\n"
        "    sr_top real,\n"
        "    sr_root real,\n"
        "    htop real,\n"
        "    hroot real,\n"
        "    hbot real,\n"
        "    a_level integer\n"
        ");\n",
        //"--ALTER TABLE a_level OWNER TO postgres;\n",
        "CREATE TABLE atmosph (\n"
        "    gid integer NOT NULL,\n"
        "    tm real NOT NULL,\n"
        "    prec_cm real,\n"
        "    rsoil_cm real,\n"
        "    rroot_cm real,\n"
        "    hcrita real DEFAULT 100000\n"
        ");\n",
        //"--ALTER TABLE atmosph OWNER TO postgres;\n",
        "CREATE TABLE atmosphstatistics (\n"
        "    gid integer NOT NULL,\n"
        "    recordcnt integer,\n"
        "    hcrits real DEFAULT 1000 NOT NULL\n"
        ");\n",
        //"--ALTER TABLE atmosphstatistics OWNER TO postgres;\n",
        "CREATE TABLE errlog (\n"
        "    gid smallint NOT NULL,\n"
        "    errordes character varying(1000)\n"
        ");\n",
        //"--ALTER TABLE errlog OWNER TO postgres;\n",
        "CREATE TABLE nod_info (\n"
        "    gid integer NOT NULL,\n"
        "    node smallint NOT NULL,\n"
        "    darcian_velocity real,\n"
        "    tm real NOT NULL,\n"
        "    depth real,\n"
        "    head real,\n"
        "    moistrue real,\n"
        "    k real,\n"
        "    c real,\n"
        "    sink real,\n"
        "    kappa real,\n"
        "    vdivkstop real,\n"
        "    temp real\n"
        ");\n",
        //"--ALTER TABLE nod_info OWNER TO postgres;\n",
        "CREATE TABLE obsnode (\n"
        "    gid integer NOT NULL,\n"
        "    tm real NOT NULL,\n"
        "    node smallint NOT NULL,\n"
        "    h real,\n"
        "    theta real,\n"
        "    flux real\n"
        ");\n",
        //"--ALTER TABLE obsnode OWNER TO postgres;\n",
        "CREATE TABLE profile (\n"
        "    gid integer NOT NULL,\n"
        "    node smallint NOT NULL,\n"
        "    x_coord real,\n"
        "    matnum smallint DEFAULT 1,\n"
        "    laynum smallint DEFAULT 1,\n"
        "    beta real,\n"
        "    ah real DEFAULT 1,\n"
        "    ak real DEFAULT 1,\n"
        "    ath real DEFAULT 1,\n"
        "    h real\n"
        ");\n",
        //"--ALTER TABLE profile OWNER TO postgres;\n",
        "CREATE TABLE profilestatistics (\n"
        "    gid integer NOT NULL,\n"
        "    depth real,\n"
        "    nodecnt smallint,\n"
        "    observercnt smallint DEFAULT 0 NOT NULL,\n"
        "    observerid smallint[]\n"
        ");\n",
        //"--ALTER TABLE profilestatistics OWNER TO postgres;\n",
        "CREATE TABLE selector (\n"
        "    gid integer NOT NULL,\n"
        "    lunit character varying(10) DEFAULT 'cm'::character varying,\n"
        "    tunit character varying(10) DEFAULT 'days'::character varying,\n"
        "    nmat smallint DEFAULT 1,\n"
        "    nlay smallint DEFAULT 1,\n"
        "    inittime real DEFAULT 0,\n"
        "    maxtime real,\n"
        "    printinterval real DEFAULT 10,\n"
        "    printtimes smallint DEFAULT 1,\n"
        "    model smallint DEFAULT 4,\n"
        "    p0 real DEFAULT 0,\n"
        "    p2h real DEFAULT '-500'::integer,\n"
        "    p2l real DEFAULT '-900'::integer,\n"
        "    p3 real DEFAULT '-16000'::integer,\n"
        "    r2h real DEFAULT 0.5,\n"
        "    r2l real DEFAULT 0.1,\n"
        "    omegac real DEFAULT 1,\n"
        "    lroot smallint DEFAULT 1,\n"
        "    lsink smallint DEFAULT 1,\n"
        "    lwlayer smallint DEFAULT 1,\n"
        "    linitw smallint DEFAULT 1,\n"
        "    maxit smallint DEFAULT 500,\n"
        "    itmin smallint DEFAULT 3,\n"
        "    itmax smallint DEFAULT 7,\n"
        "    tolth real DEFAULT 0.001,\n"
        "    tolh real DEFAULT 1,\n"
        "    ha real DEFAULT 0.000001,\n"
        "    hb real DEFAULT 10000,\n"
        "    dt real DEFAULT 0.001,\n"
        "    dtmin real DEFAULT 0.00001,\n"
        "    dtmax real DEFAULT 0.5,\n"
        "    dmul real DEFAULT 1.3,\n"
        "    dmul2 real DEFAULT 0.7,\n"
        "    rootgrowthcnt smallint,\n"
        "    matdata real[],\n"
        "    printtimedata real[],\n"
        "    rootdate real[],\n"
        "    rootlength real[],\n"
        "    poptm real[],\n"
        "    status character varying(20) DEFAULT 'todo'::character varying,\n"
        "    dtandtm character varying(80),\n"
        "    caltm numeric(16,13)\n"
        ");\n",
        //"--ALTER TABLE selector OWNER TO postgres;\n",
        "CREATE TABLE soilcolumnpositions (\n"
        "    gid integer NOT NULL,\n"
        "    x real,\n"
        "    y real,\n"
        "    landuse character varying(50),\n"
        "    geom geometry(point,4326)\n"
        ");\n",
        //"--ALTER TABLE soilcolumnpositions OWNER TO postgres;\n",
        "CREATE TABLE t_level (\n"
        "    gid integer NOT NULL,\n"
        "    tm real NOT NULL,\n"
        "    vtop real,\n"
        "    vroot real,\n"
        "    vbot real,\n"
        "    sv_top real,\n"
        "    sv_root real,\n"
        "    sv_bot real,\n"
        "    vol real,\n"
        "    s_infil real,\n"
        "    s_evap real,\n"
        "    rtop real,\n"
        "    rroot real,\n"
        "    sr_top real,\n"
        "    sr_root real,\n"
        "    htop real,\n"
        "    hroot real,\n"
        "    hbot real,\n"
        "    runoff real,\n"
        "    s_runoff real,\n"
        "    t_level integer,\n"
        "    cum_wtrans real,\n"
        "    snowlayer real\n"
        ");\n",
        //"--ALTER TABLE t_level OWNER TO postgres;\n",
        "CREATE TABLE waterbalance (\n"
        "    gid integer NOT NULL,\n"
        "    tm real NOT NULL,\n"
        "    subregion smallint NOT NULL,\n"
        "    length real,\n"
        "    w_volume real,\n"
        "    in_flow real,\n"
        "    h_mean real,\n"
        "    top_flux real,\n"
        "    bot_flux real,\n"
        "    watbalt real,\n"
        "    watbalr real\n"
        ");\n",
        //"--ALTER TABLE waterbalance OWNER TO postgres;\n",
        "ALTER TABLE ONLY a_level\n"
        "    ADD CONSTRAINT a_level_gid_tm_pk PRIMARY KEY (gid, tm);\n",
        "ALTER TABLE ONLY atmosph\n"
        "    ADD CONSTRAINT atmosph_pk PRIMARY KEY (gid, tm);\n",
        "ALTER TABLE ONLY atmosphstatistics\n"
        "    ADD CONSTRAINT atmosphstatistics_pk PRIMARY KEY (gid);\n",
        "ALTER TABLE ONLY errlog\n"
        "    ADD CONSTRAINT errlogpk PRIMARY KEY (gid);\n",
        "ALTER TABLE ONLY obsnode\n"
        "    ADD CONSTRAINT obsnode_gid_tm_node_pk PRIMARY KEY (gid, tm, node);\n",
        "ALTER TABLE ONLY nod_info\n"
        "    ADD CONSTRAINT pk PRIMARY KEY (gid, tm, node);\n",
        "ALTER TABLE ONLY profile\n"
        "    ADD CONSTRAINT profile_pk PRIMARY KEY (gid, node);\n",
        "ALTER TABLE ONLY profilestatistics\n"
        "    ADD CONSTRAINT profilestatistics_pk PRIMARY KEY (gid);\n",
        "ALTER TABLE ONLY selector\n"
        "    ADD CONSTRAINT selector_pkey PRIMARY KEY (gid);\n",
        "ALTER TABLE ONLY soilcolumnpositions\n"
        "    ADD CONSTRAINT soilcolumnpositions_pkey PRIMARY KEY (gid);\n",
        "ALTER TABLE ONLY t_level\n"
        "    ADD CONSTRAINT t_level_pk PRIMARY KEY (gid, tm);\n",
        "ALTER TABLE ONLY waterbalance\n"
        "    ADD CONSTRAINT waterbalance_pkey PRIMARY KEY (gid, tm, subregion);\n",
        "CREATE INDEX a_level_gid_idx ON a_level USING btree (gid);\n",
        "CREATE INDEX atmosph_gid_idx ON atmosph USING btree (gid);\n",
        "CREATE INDEX nod_info_gid_idx ON nod_info USING btree (gid);\n",
        "CREATE INDEX profile_gid_idx ON profile USING btree (gid);\n",
        "CREATE INDEX t_level_gid_idx ON t_level USING btree (gid);\n",
        "[PARTITION TABLE SEGMENT]\n",
        "CREATE OR REPLACE FUNCTION public.errlog_insert_trig()\n"
        "  RETURNS trigger AS\n"
        "$BODY$\n"
        "BEGIN\n"
        "   if (new.errordes='do not converge') then\n"
        "      update selector set status='not converged' where gid=new.gid;\n"
        "   end if;\n"
        "   return null;\n"
        "end;\n"
        "$BODY$\n"
        "  LANGUAGE plpgsql;\n"
        //"--ALTER FUNCTION public.errlog_insert_trig()\n"
        //"--  OWNER TO postgres;\n",
        "CREATE TRIGGER errlog_insert\n"
        "AFTER INSERT\n"
        "ON public.errlog\n"
        "FOR EACH ROW\n"
        "EXECUTE PROCEDURE public.errlog_insert_trig();\n"//,
        //"--REVOKE ALL ON SCHEMA public FROM postgres;\n",
        //"--GRANT ALL ON SCHEMA public TO postgres;\n",
        //"--GRANT ALL ON SCHEMA public TO PUBLIC;"
    };
};

#endif // DATABASESQLCOMMANDS_H
