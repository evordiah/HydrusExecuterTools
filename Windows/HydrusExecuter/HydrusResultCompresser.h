
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

#ifndef HYDRUSRESULTCOMPRESSER_H
#define HYDRUSRESULTCOMPRESSER_H
#include <string>
#include <memory>

class A_LevelObject;
class T_LevelObject;
class NodeInfoObject;
class BalanceObject;
class Obs_NodeObject;

class HydrusResultCompresser
{
public:
    static bool Compress(const std::string& sourcepath,const std::string& tofilename,bool RemoveSrcFiles=true);
    static bool Compress(const A_LevelObject& aobj,const T_LevelObject& tobj,const NodeInfoObject& nobj,
                         const BalanceObject& bobj, const std::string& tofilename);
    static bool Compress(const A_LevelObject& aobj,const T_LevelObject& tobj,const NodeInfoObject& nobj,
                         const BalanceObject& bobj,const Obs_NodeObject&oobj, const std::string& tofilename);
    static bool UnCompress(const std::string& filename,const std::string& topath);
    static bool IsConverged(const std::string& filename);
    static int GetPartCount(const std::string& filename);
    static std::unique_ptr<A_LevelObject> ExtractALevel(const std::string& filename);
    static std::unique_ptr<T_LevelObject> ExtractTLevel(const std::string& filename);
    static std::unique_ptr<NodeInfoObject> ExtractNodeInfo(const std::string& filename);
    static std::unique_ptr<BalanceObject> ExtractBalance(const std::string& filename);
    static std::unique_ptr<Obs_NodeObject> ExtractObsNode(const std::string& filename);
protected:

private:

};

#endif // HYDRUSRESULTCOMPRESSER_H
