
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

#include "exporthydrusoutputfile.h"
#include "HydrusResultCompresser.h"
#include "aleveldatabaseobject.h"
#include "tleveldatabaseobject.h"
#include "nodinfodatabaseobject.h"
#include "balancedatabaseobject.h"
#include "obsnodedatabaseobject.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

ExportHydrusOutputFile::ExportHydrusOutputFile(std::shared_ptr<QSqlQuery> qry)
{
    _pqry=qry;
    _valid=true;
}

ExportHydrusOutputFile::~ExportHydrusOutputFile()
{
	if (_pqry)
	{
		_pqry->finish();
	}
	_pqry = nullptr;
}

bool ExportHydrusOutputFile::Execute(const std::string &value)
{
    if(_valid)
    {
		std::unique_ptr<A_LevelObject> paobj = GetALevelSql();
		std::unique_ptr<T_LevelObject> ptobj = GetTLevelSql();
		std::unique_ptr<NodeInfoObject> pnobj = GetNodinfoSql();
		std::unique_ptr<BalanceObject> pbobj = GetBalancesql();
        std::unique_ptr<Obs_NodeObject> poobj = GetObsNodesql();
        if(!paobj || !ptobj || !pnobj || !pbobj)
        {
            _valid=false;
            return false;
        }
        boost::filesystem::path p=value;
        if(boost::filesystem::is_directory(p))
        {
			paobj->SaveAsA_LevelFile(value);
            ptobj->SaveAsT_LevelFile(value);
            pnobj->SaveAsNod_InfFile(value);
            pbobj->SaveAsBalancefFile(value);
            if (poobj)
			{
                poobj->SaveAsObs_NodeFile(value);
			}
        }
        else
        {
            if(poobj)
            {
                _valid=HydrusResultCompresser::Compress(*paobj,*ptobj,*pnobj,*pbobj,*poobj,value);
            }
            else
            {
                _valid=HydrusResultCompresser::Compress(*paobj,*ptobj,*pnobj,*pbobj,value);
            }
        }
    }
    return _valid;
}

void ExportHydrusOutputFile::Gid(int value)
{
    _gid = value;
}

std::unique_ptr<A_LevelObject> ExportHydrusOutputFile::GetALevelSql()
{
    std::unique_ptr<ALevelDataBaseObject> result(new ALevelDataBaseObject(_gid, *_pqry));
    if(*(result.get()))
    {
        return std::unique_ptr<A_LevelObject>((A_LevelObject*)(result.release()));
    }
    return std::unique_ptr<A_LevelObject>();
}

std::unique_ptr<T_LevelObject> ExportHydrusOutputFile::GetTLevelSql()
{
    std::unique_ptr<TLevelDataBaseObject> result(new TLevelDataBaseObject(_gid, *_pqry));
    if(*(result.get()))
    {
        return std::unique_ptr<T_LevelObject>((T_LevelObject*)(result.release()));
    }
    return std::unique_ptr<T_LevelObject>();
}

std::unique_ptr<NodeInfoObject> ExportHydrusOutputFile::GetNodinfoSql()
{
    std::unique_ptr<NodinfoDataBaseObject> result(new NodinfoDataBaseObject(_gid, *_pqry));
    if(*(result.get()))
    {
        return std::unique_ptr<NodeInfoObject>((NodeInfoObject*)(result.release()));
    }
    return std::unique_ptr<NodeInfoObject>();
}

std::unique_ptr<BalanceObject> ExportHydrusOutputFile::GetBalancesql()
{
    std::unique_ptr<BalanceDataBaseObject> result(new BalanceDataBaseObject(_gid, *_pqry));
    if(*(result.get()))
    {
        return std::unique_ptr<BalanceObject>((BalanceObject*)(result.release()));
    }
    return std::unique_ptr<BalanceObject>();
}

std::unique_ptr<Obs_NodeObject> ExportHydrusOutputFile::GetObsNodesql()
{
    std::unique_ptr<ObsNodeDataBaseObject> result(new ObsNodeDataBaseObject(_gid, *_pqry));
    if(*(result.get()))
    {
        return std::unique_ptr<Obs_NodeObject>((Obs_NodeObject*)(result.release()));
    }
    return std::unique_ptr<Obs_NodeObject>();
}

