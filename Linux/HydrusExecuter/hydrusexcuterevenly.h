
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

#ifndef HYDRUSEXCUTEREVENLY_H
#define HYDRUSEXCUTEREVENLY_H
#include "HydrusExcuter.h"
#include <memory>
#include <queue>
#include <vector>

class HydrusExcuterEvenly : public HydrusExcuter
{
public:
    HydrusExcuterEvenly(const std::string& exepath,std::vector<int>& gids);
 protected:
    virtual bool GetId(int & gid);
    std::shared_ptr<std::queue<int>> _pgidqueue;
};

#endif // HYDRUSEXCUTEREVENLY_H
