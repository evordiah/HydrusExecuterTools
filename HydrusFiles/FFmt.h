
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

#ifndef FFMT_H
#define FFMT_H
#include <iomanip>
#include <ostream>
#include <cmath>
#include <limits>

class TLevelObject;

namespace fwzformat
{
    struct ffmt_proxy
    {
        explicit ffmt_proxy(std::ostream &os) : os(os), _exponentwidth(0), _bFortranFormat(false)
        {
        }
        explicit ffmt_proxy(std::ostream &os, int exponetwidth, bool bFortranFormat = true) : os(os),
                                                                                              _exponentwidth(exponetwidth), _bFortranFormat(bFortranFormat)
        {
        }
        template <typename Rhs>
        friend std::ostream &operator<<(ffmt_proxy const &q,
                                        Rhs const &rhs)
        {
            return q.os << rhs;
        }

        friend std::ostream &operator<<(ffmt_proxy const &q,
                                        std::string const &rhs)
        {
            return q.os << "'" << rhs << "'";
        }

        friend std::ostream &operator<<(ffmt_proxy const &q,
                                        char const *rhs)
        {
            return q.os << "'" << rhs << "'";
        }

        friend std::ostream &operator<<(const ffmt_proxy &q, const float &rhs);
        friend std::ostream &operator<<(const ffmt_proxy &q, const double &rhs);

    private:
        std::ostream &os;
        int _exponentwidth;
        bool _bFortranFormat;
    };

    //extern struct quote_creator { } quoter;
    //extern struct fortranE2_creator {} fortranE2;
    //extern struct fortranE3_creator {} fortranE3;
    //extern struct SE3_creator {} SE3;
    //extern struct SqlValueExpress_creator {} SqlValueExpression;
    struct quote_creator
    {
    };
    struct fortranE2_creator
    {
    };
    struct fortranE3_creator
    {
    };
    struct SE3_creator
    {
    };
    struct SqlValueExpress_creator
    {
    };
    extern struct quote_creator quoter;
    extern struct fortranE2_creator fortranE2;
    extern struct fortranE3_creator fortranE3;
    extern struct SE3_creator SE3;
    extern struct SqlValueExpress_creator SqlValueExpression;
    ffmt_proxy operator<<(std::ostream &os, quote_creator);
    ffmt_proxy operator<<(std::ostream &os, fortranE2_creator);
    ffmt_proxy operator<<(std::ostream &os, fortranE3_creator);
    ffmt_proxy operator<<(std::ostream &os, SE3_creator);
    ffmt_proxy operator<<(std::ostream &os, SqlValueExpress_creator);
    std::ostream &operator<<(const ffmt_proxy &q, const float &rhs);
    std::ostream &operator<<(const ffmt_proxy &q, const double &rhs);
    template <typename Rhs>
    std::ostream &operator<<(ffmt_proxy const &q, Rhs const &rhs);
    std::ostream &operator<<(ffmt_proxy const &q, std::string const &rhs);
    std::ostream &operator<<(ffmt_proxy const &q, char const *rhs);
} // namespace fwzformat

#endif // FFMT_H
