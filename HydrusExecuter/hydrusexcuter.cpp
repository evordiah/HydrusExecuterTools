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
#include <thread>
#include "hydrusexcuter.h"
#include "applicationparametermanager.h"
#include "pqdbconn.h"

HydrusExecuter::HydrusExecuter()
{
    _grpid = 0;
    _pqry = nullptr;
    _dbConnName = "";
    _exepath = ApplicationParameterManager::Instance().HydursExecutingPath();
}

bool HydrusExecuter::InitialThread()
{
    if (!GetDatabase() && ApplicationParameterManager::Instance().RunOnServer())
    {
        std::cerr << "Can't connect database!" << std::endl;
        return false;
    }
    auto tid = std::this_thread::get_id();
    std::stringstream strbld;
    strbld << "hydrus" << tid;
    _currentprocesspath = std::filesystem::absolute(_exepath / strbld.str());
    if (!std::filesystem::exists(_currentprocesspath))
    {
        if (!std::filesystem::create_directories(_currentprocesspath))
        {
            return false;
        }
    }
    return true;
}

void HydrusExecuter::ClearTempFile()
{
    std::filesystem::directory_iterator beg(_currentprocesspath);
    std::filesystem::directory_iterator end;
    for (; beg != end; ++beg)
    {
        if (beg->is_directory())
        {
            std::filesystem::remove_all(beg->path());
        }
        else
        {
            std::filesystem::remove(beg->path());
        }
    }
    if (_HydrusFilesManager)
    {
        _HydrusFilesManager = nullptr;
    }
}

void HydrusExecuter::CleanThread()
{
    _pqry = nullptr;
    DBConnManager::GetInstance().RemoveConnection(_dbConnName);
    std::filesystem::remove_all(_currentprocesspath);
}

bool HydrusExecuter::GetDatabase()
{
    if (_dbConnName.empty() && !_pqry)
    {
        std::stringstream strbld("Hydrus_Executer_thread_DBConn_");
        strbld << std::this_thread::get_id();
        _dbConnName = strbld.str();
        auto pConn = DBConnManager::GetInstance().GetConnection()->Clone();
        auto pqConn = DBConnManager::GetInstance().MakeConnection(_dbConnName, pConn);
        _pqry = pqConn->GetConn();
    }
    return _pqry != nullptr;
}

void HydrusExecuter::operator()()
{
    Execute();
}

void HydrusExecuter::Execute()
{
    if (!InitialThread())
    {
        return;
    }
    ApplicationParameterManager::RegisterThreadStarting();
    int gid;
    while (GetId(gid))
    {
        if (PrepareParameterFile(gid))
        {
            if (ExecuteHydrus(gid))
            {
                auto r = _HydrusFilesManager->GetImportResultFilesSQlStatement();
                if (r)
                {
                    if (_HydrusFilesManager->HasErr())
                    {
                        std::string tmp = _HydrusFilesManager->ErrMessage();
                        EnterFail(gid, tmp);
                    }
                    else
                    {
                        EnterComplete(gid, *r);
                    }
                }
                else
                {
                    std::string tmp("Fail to read Hydrus output files!");
                    EnterFail(gid, tmp);
                }
            }
            ClearTempFile();
        }
    }
    CleanThread();
    ApplicationParameterManager::RegisterThreadFinished();
}
