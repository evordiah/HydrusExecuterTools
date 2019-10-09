
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

#ifndef OBSNODEDATABASEOBJECT_H
#define OBSNODEDATABASEOBJECT_H
#include "obs_nodeobject.h"

class QSqlQuery;

class ObsNodeDataBaseObject : public Obs_NodeObject
{
public:
    ObsNodeDataBaseObject(int gid,QSqlQuery& qry);
protected:
    void Time(int index,float val)
    {
        _data[index*_nFiledCnt]=val;
    }

    void H(int index,int nodeindex,float val)
    {
        _data[index*_nFiledCnt+nodeindex*3+1]=val;
    }

    void Theta(int index,int nodeindex,float val)
    {
        _data[index*_nFiledCnt+nodeindex*3+2]=val;
    }

    void Flux(int index,int nodeindex,float val)
    {
        _data[index*_nFiledCnt+nodeindex*3+3]=val;
    }
};

#endif // OBSNODEDATABASEOBJECT_H
