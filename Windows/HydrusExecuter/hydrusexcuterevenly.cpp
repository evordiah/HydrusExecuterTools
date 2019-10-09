
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

#include "hydrusexcuterevenly.h"

HydrusExcuterEvenly::HydrusExcuterEvenly(const std::string &exepath, std::vector<int> &gids)
    :HydrusExcuter(exepath)
{
    _pgidqueue.reset(new std::queue<int>());
    for(auto e :gids)
    {
        _pgidqueue->push(e);
    }
}

bool HydrusExcuterEvenly::GetId(int &gid)
{
    if(!_pgidqueue->empty())
    {
        gid=_pgidqueue->front();
        _pgidqueue->pop();
        return true;
    }
    return false;
}
