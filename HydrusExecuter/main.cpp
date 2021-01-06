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
#include "asio.hpp"
#include "taskcontroller.h"
#include "applicationparametermanager.h"
#include "serverprocessindicator.h"
#include "clientprocessindicator.h"
#include "hydrus_tcp_server.h"
#include "commandlineparser.h"

int main(int argc, char *argv[])
{
    using namespace std;
    CommandLineParser parser(argc, argv);
    if (!parser)
    {
        return 0;
    }
    asio::io_context io;
    try
    {
        if (ApplicationParameterManager::Instance().RunOnServer())
        {
            std::shared_ptr<ServerProcessIndicator> serverprocessIndicatorptr = std::make_shared<ServerProcessIndicator>(io);
            std::shared_ptr<HydrusTcpServer> hydrusTcpServerptr = std::make_shared<HydrusTcpServer>(io);
            std::future<void> f1 = std::async(std::launch::async, [hydrusTcpServerptr] { hydrusTcpServerptr->listen(ApplicationParameterManager::Instance().HostPort()); });
            std::future<void> f2 = std::async(std::launch::async, [serverprocessIndicatorptr] { serverprocessIndicatorptr->start(1000); });
            TaskController::GetController().Run();
            serverprocessIndicatorptr->stop();
            hydrusTcpServerptr->stop();
            f1.get();
            f2.get();
        }
        else
        {
            std::shared_ptr<ClientProcessIndicator> clientprocessIndicatorptr = std::make_shared<ClientProcessIndicator>(io);
            std::future<void> f = std::async(std::launch::async, [clientprocessIndicatorptr] { clientprocessIndicatorptr->start(1000); });
            TaskController::GetController().Run();
            clientprocessIndicatorptr->stop();
            f.get();
        }
        TaskController::GetController().Release();
        ApplicationParameterManager::Instance().Release();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
