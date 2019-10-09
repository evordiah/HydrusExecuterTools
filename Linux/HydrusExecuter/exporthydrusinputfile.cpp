
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

#include "exporthydrusinputfile.h"
#include "HydrusInputCompresser.h"
#include "atmosphdatabaseobject.h"
#include "profiledatabaseobject.h"
#include "selectordatabaseobject.h"
#include <string>
#include <regex>
#include <sstream>
#include <QSqlQuery>
#include <QFileInfo>
#include <iostream>

ExportHydrusInputFile::ExportHydrusInputFile(std::shared_ptr<QSqlQuery> qry)
{
    _pqry=qry;
    _valid=true;
}

ExportHydrusInputFile::~ExportHydrusInputFile()
{
}

bool ExportHydrusInputFile::Execute(const std::string &value)
{
    std::unique_ptr<AtmosphObject> paobj= GetAtmosphsql();
    std::unique_ptr<ProfileObject> ppobj=GetProfilesql();
    std::unique_ptr<SelectorObject> psobj=GetSelectorsql();
    if(_valid)
    {
		QFileInfo qfinf(value.c_str());
		if(qfinf.isDir())
        {
            try
            {
                paobj->SaveAsAtmosphFile(value);
                ppobj->SaveAsProfileFile(value);
                psobj->SaveAsSelectorFile(value);
            }
            catch(...)
            {
                _valid=false;
            }
        }
        else
        {
            _valid=HydrusInputCompresser::Compress(*psobj,*paobj,*ppobj,value);
        }
    }

    return _valid;
}

std::unique_ptr<AtmosphObject> ExportHydrusInputFile::GetAtmosphsql()
{
    return std::unique_ptr<AtmosphObject>((AtmosphObject*)(new AtmosphDatabaseObject(_gid,*_pqry)));
}

std::unique_ptr<ProfileObject> ExportHydrusInputFile::GetProfilesql()
{
    return std::unique_ptr<ProfileObject>((ProfileObject*)(new ProfileDataBaseObject(_gid,*_pqry)));
}

std::unique_ptr<SelectorObject> ExportHydrusInputFile::GetSelectorsql()
{
    return std::unique_ptr<SelectorObject>((SelectorObject*)(new SelectorDataBaseObject(_gid,*_pqry)));
}
