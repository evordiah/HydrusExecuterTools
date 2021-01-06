
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

#include "FFmt.h"

struct fwzformat::quote_creator fwzformat::quoter;
struct fwzformat::fortranE2_creator fwzformat::fortranE2;
struct fwzformat::fortranE3_creator fwzformat::fortranE3;
struct fwzformat::SE3_creator fwzformat::SE3;
struct fwzformat::SqlValueExpress_creator fwzformat::SqlValueExpression;


fwzformat::ffmt_proxy fwzformat::operator<<(std::ostream &os, fwzformat::quote_creator)
{
    return ffmt_proxy(os);
}

fwzformat::ffmt_proxy fwzformat::operator<<(std::ostream &os, fwzformat::fortranE2_creator)
{
    return ffmt_proxy(os,2);
}

fwzformat::ffmt_proxy fwzformat::operator<<(std::ostream &os, fwzformat::fortranE3_creator)
{
    return ffmt_proxy(os,3);
}

fwzformat::ffmt_proxy fwzformat::operator<<(std::ostream &os, fwzformat::SE3_creator)
{
    return ffmt_proxy(os,3,false);
}

fwzformat::ffmt_proxy fwzformat::operator<<(std::ostream &os, fwzformat::SqlValueExpress_creator)
{
    return ffmt_proxy(os);
}

std::ostream &fwzformat::operator<<(const fwzformat::ffmt_proxy &q, const float &rhs)
{
    if(!q._exponentwidth)
    {
        return q.os<<rhs;
    }
    char E='E';
    int width=q.os.width();
    float val=rhs;
    int exp=0;
    int fac=1;
    if(val<0)
    {
        val*=-1;
        fac=-1;
    }
    if(val)
    {
        for(; val>=1.0; exp++)
            val/=10;
        for(; (0.1-val)>std::numeric_limits<float>::epsilon(); exp--)
            val*=10;
        val*=fac;
    }
    int ew=q._exponentwidth==2?4:5;
    if(width>=ew)
    {
        width-=ew;
    }
    char f=q.os.fill();
    if(!q._bFortranFormat)
    {
        if(val)
        {
            val*=10;
            exp--;
        }
        E='e';
    }
    q.os<<std::setw(width)<<val<<E<<std::showpos
       <<std::setw(q._exponentwidth+1)<<std::setfill('0')<<std::internal<<exp;
    q.os.unsetf(std::ios_base::internal);
    return q.os<<std::setfill(f)<<std::noshowpos;

}

std::ostream &fwzformat::operator<<(const fwzformat::ffmt_proxy &q, const double &rhs)
{
    if(!q._exponentwidth)
    {
        return q.os<<rhs;
    }
    char E='E';
    int width=q.os.width();
    double val=rhs;
    int exp=0;
    int fac=1;
    if(val<0)
    {
        val*=-1;
        fac=-1;
    }
    if(val)
    {
        for(; val>=1.0; exp++)
            val/=10;
        for(; (0.1-val)>std::numeric_limits<double>::epsilon(); exp--)
            val*=10;
        val*=fac;
    }
    int ew=q._exponentwidth==2?4:5;
    if(width>=ew)
    {
        width-=ew;
    }
    char f=q.os.fill();
    if(!q._bFortranFormat)
    {
        if(val)
        {
            val*=10;
            exp--;
        }
        E='e';
    }
    q.os<<std::setw(width)<<val<<E<<std::showpos
       <<std::setw(q._exponentwidth+1)<<std::setfill('0')<<std::internal<<exp;
    q.os.unsetf(std::ios_base::internal);
    return q.os<<std::setfill(f)<<std::noshowpos;
}



