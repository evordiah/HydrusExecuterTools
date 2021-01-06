
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

#ifndef COMMITJOB_H
#define COMMITJOB_H

#include <memory>
#include <string>
#include <list>

namespace pqxx
{
    class connection;
}

class CommitJob
{
public:
    CommitJob(int gid, unsigned int grpid,
              std::string &val, bool bErr = false);
    CommitJob(int gid, unsigned int grpid);
    void operator()(pqxx::connection *qry);
private:
    void operator()();
    void operator()(pqxx::connection &qry);
    void CommitError();
    void CommitError(pqxx::connection &qry);
    void CommitResult();
    void CommitResult(pqxx::connection &qry);
    int SendResult(int gid, unsigned int grpid);
    int SendErr(int gid, unsigned int grpid);
    void SplitSqlState(const std::string &state, std::list<std::string> &sqls) const;
    void SplitInsertSqlState(const std::string &state, std::list<std::string> &sqls) const;
    void ImportFromFile(pqxx::connection &qry);

private:
    int _gid;
    unsigned int _grpid;
    bool _bSaveInFile;
    std::shared_ptr<std::string> _presult;
    std::shared_ptr<std::string> _perr;
};

#endif // COMMITJOB_H
