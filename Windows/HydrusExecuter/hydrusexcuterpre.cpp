
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

#include "hydrusexcuterpre.h"

HydrusExcuterPre::HydrusExcuterPre(const std::string &exepath):HydrusExcuter(exepath)
{
}

bool HydrusExcuterPre::GetId(int &gid)
{
    bool result=true;
    {
        std::lock_guard<std::mutex> lg(gidsMutex);
        if(!gids.empty())
        {
            gid=gids.front();
            gids.pop();
        }
        else
        {
            result=false;
        }
    }
    return result;
}
