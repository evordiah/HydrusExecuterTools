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

#ifndef NODEJOB_H
#define NODEJOB_H

#include <string>
#include <memory>
#include "abstractjob.h"

class NodeJob : public AbstractJob
{
public:
    NodeJob(int gid, unsigned int grpid,
            std::string &val, bool berr = false);
    virtual void operator()();

private:
    void CommitError();
    void CommitResult();
    int SendResult(int gid, unsigned int grpid);
    int SendErr(int gid, unsigned int grpid);
    void ImportFromFile();

private:
    int _gid;
    unsigned int _grpid;
    bool _bSaveInFile;
    std::shared_ptr<std::string> _presult;
    std::shared_ptr<std::string> _perr;
};

#endif // NODEJOB_H
