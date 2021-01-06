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

#ifndef HYDRUSEXCUTER_H
#define HYDRUSEXCUTER_H

#include <memory>
#include <string>
#include <filesystem>
#include "HydrusParameterFilesManager.h"

namespace pqxx
{
    class connection;
}
class HydrusExecuter
{
public:
    HydrusExecuter();
    HydrusExecuter(const HydrusExecuter& rhs)=delete ;
    HydrusExecuter(HydrusExecuter&& rhs)=delete ;
    HydrusExecuter& operator=(const HydrusExecuter& rhs)=delete ;
    HydrusExecuter& operator=(HydrusExecuter&& rhs)=delete ;
    virtual ~HydrusExecuter()=default;
    void operator()();
    virtual void Execute();
protected:
    bool InitialThread();
    void ClearTempFile();
    void CleanThread();
    virtual bool ExecuteHydrus(int gid)=0;
    virtual bool GetId(int & gid)=0;
    virtual bool PrepareParameterFile(int gid)=0;
    virtual bool EnterRunning(int gid)=0;
    virtual bool EnterFail(int gid, std::string& reason)=0;
    virtual bool EnterTimeout(int gid)=0;
    virtual bool EnterComplete(int gid, std::string& result)=0;
private:
    bool GetDatabase();
protected:
    unsigned int _grpid;
    std::filesystem::path _exepath;
    std::filesystem::path _currentprocesspath;
    std::string _dbConnName;
    pqxx::connection* _pqry;
    std::unique_ptr<HydrusParameterFilesManager> _HydrusFilesManager;
};

#endif // HYDRUSEXCUTER_H
