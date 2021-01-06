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

#include <memory>
#include "applicationparametermanager.h"
#include "taskcontroller.h"
#include "nodetaskcontroller.h"
#include "servertaskcontroller.h"

std::unique_ptr<TaskController> TaskController::pcontroler(nullptr);

TaskController &TaskController::GetController()
{
    if(!pcontroler)
    {
        if(ApplicationParameterManager::Instance().RunOnServer())
        {
            pcontroler = std::make_unique<ServerTaskController>();
        }
        else
        {
            pcontroler = std::make_unique<NodeTaskController>();
        }
    }
    return *pcontroler;
}




