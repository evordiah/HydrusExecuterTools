
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

#include <limits>
#include <cctype>
#include <memory>
#include <cstring>
#include <regex>
#include <algorithm>
#include "Stringhelper.h"


std::pair<size_t,size_t> Stringhelper::findArgEscapes()
{
    struct
    {
        int min_escape;
        size_t s;
        size_t e;
    } d;

    const std::string& s=_str;
    auto itb=s.cbegin();
    auto ite=s.cend();

    d.min_escape =std::numeric_limits<int>::max();
    d.s=std::string::npos;
    d.e=std::string::npos;
    size_t tmp=0;
    auto itc = itb;
    while (itc != ite)
    {
        while (itc != ite && *itc != '%')
        {
            ++itc;
        }
        if (itc == ite)
        {
            break;
        }

        tmp=itc-itb;
        if (++itc == ite)
        {
            break;
        }

        int escape = std::isdigit(*itc)?(*itc-'0'):-1;
        if (escape == -1)
        {
            continue;
        }
        ++itc;

        if (itc != ite)
        {
            int next_escape = std::isdigit(*itc)?(*itc-'0'):-1;
            if (next_escape != -1)
            {
                escape = (10 * escape) + next_escape;
                ++itc;
            }
        }

        if (escape > d.min_escape)
        {
            continue;
        }

        if (escape < d.min_escape)
        {
            d.min_escape = escape;
        }
        d.s=tmp;
        d.e=itc-itb;
    }
    return std::make_pair(d.s,d.e);
}

void Stringhelper::replaceArgEscapes(const std::string& s)
{
    auto p=findArgEscapes();
    if(p.second==std::string::npos)
    {
        return;
    }
    std::string t=_str.substr(0,p.first);
    t.append(s);
    if(p.second<_str.size())
    {
        t.append(_str.substr(p.second));
    }
    _str=t;
}

void Stringhelper::replaceCaseSensitive(const std::string &before, const std::string &after)
{
    if(_str.empty() || before.empty())
    {
        return;
    }
    size_t s=0;
    size_t e=0;
    size_t nc=before.size();
    size_t nc1=after.size();
    while ((e=_str.find(before,s))!=std::string::npos)
    {
        _str.replace(e,nc,after);
        s=e+nc1;
    }
}

void Stringhelper::replaceInCaseSensitive(const std::string &before, const std::string &after)
{
    std::regex rg(before,std::regex_constants::icase);
    std::string t=std::regex_replace(_str,rg,after);
    _str=t;
}

Stringhelper& Stringhelper::arg(long long a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(unsigned long long a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(long a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(unsigned long a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(int a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(unsigned int a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(short a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(unsigned short a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(double a)
{
    replaceArgEscapes(std::to_string(a));
    return *this;
}
Stringhelper& Stringhelper::arg(char a)
{
    replaceArgEscapes(std::string(1,a));
    return *this;
}
Stringhelper& Stringhelper::arg(unsigned char a)
{
    replaceArgEscapes(std::string(1,a));
    return *this;
}
Stringhelper& Stringhelper::arg(const std::string &a)
{
    replaceArgEscapes(a);
    return *this;
}

Stringhelper &Stringhelper::simplified()
{
    if (_str.empty())
        return *this;
    auto itsrc = _str.cbegin();
    auto itend = _str.cend();
    std::unique_ptr<char[]> ptstr(new char[_str.size()+1]);
    auto dst = ptstr.get();
    auto ptr = dst;
    while(true)
    {
        while (itsrc != itend && std::isspace(*itsrc))
        {
            ++itsrc;
        }
        while (itsrc != itend && !std::isspace(*itsrc))
        {
            *ptr++ = *itsrc++;
        }
        if (itsrc == itend)
        {
            break;
        }
        *ptr++ = ' ';
    }
    *ptr=0;
    _str=dst;
    if(!_str.empty() && _str.back()==' ')
    {
        _str.pop_back();
    }
    return *this;
}

Stringhelper &Stringhelper::trimmed()
{
    auto itbegin = _str.cbegin();
    auto itend = _str.cend();
    while (itbegin < itend && std::isspace(*itbegin))
    {
        itbegin++;
    }
    if (itbegin < itend)
    {
        while (itbegin < itend && std::isspace(itend[-1]))
        {
            itend--;
        }
    }

    if (itbegin != _str.cbegin() || itend != _str.cend())
    {
        _str=std::string(itbegin,itend);
    }
    return *this;
}

std::vector<std::string> Stringhelper::split(const std::string &sep) const
{
    std::vector<std::string> list;
    const size_t separatorSize=sep.size();
    if(separatorSize==0 || _str.size()<separatorSize)
    {
        list.push_back(_str);
        return list;
    }
    size_t start = 0;
    size_t end;
    while ((end = _str.find(sep, start)) != std::string::npos)
    {
        if (start != end)
        {
            list.push_back(_str.substr(start, end - start));
        }
        start = end + separatorSize;
    }
    if (start != _str.size())
    {
        list.push_back(_str.substr(start));
    }
    return list;
}

std::vector<std::string> Stringhelper::split(const char sep) const
{
    std::vector<std::string> list;
    if(_str.empty() || sep==0)
    {
        list.push_back(_str);
        return list;
    }
    size_t start = 0;
    size_t end;
    while ((end = _str.find_first_of(sep, start)) != std::string::npos)
    {
        if (start != end)
        {
            list.push_back(_str.substr(start, end - start));
        }
        start = end + 1;
    }
    if (start != _str.size())
    {
        list.push_back(_str.substr(start));
    }
    return list;
}

bool Stringhelper::startsWith(const std::string &s,bool bcaseSensitive) const
{
    size_t ns=s.size();
    if(_str.empty())
    {
        return ns==0;
    }

    if (ns > _str.size())
    {
        return false;
    }
    const char *h = _str.c_str();
    const char *n = s.c_str();

    if (bcaseSensitive)
    {
        if (h == n)
        {
            return true;
        }
        return std::strncmp(h, n, ns) == 0;
    }

    for (size_t i = 0; i < ns; ++i)
    {
        if (std::tolower(h[i]) != std::tolower(n[i]))
        {
            return false;
        }
    }

    return true;
}

bool Stringhelper::startsWith(const char c, bool bcaseSensitive) const
{
    if(_str.empty())
    {
        return false;
    }
    if (bcaseSensitive)
    {
        if (_str[0] != c)
        {
            return false;
        }
    }
    else
    {
        if (std::tolower(_str[0]) != std::tolower(c))
        {
            return false;
        }
    }
    return true;
}

Stringhelper &Stringhelper::replace(const char before, const char after, bool bCaseSensitive)
{
    auto s=_str.begin();
    auto e=_str.end();
    if(bCaseSensitive)
    {
        while(s!=e)
        {
            if(*s==before)
            {
                *s=after;
            }
            s++;
        }
    }
    else
    {
        while(s!=e)
        {
            if(std::tolower(*s)==std::tolower(before))
            {
                *s=after;
            }
            s++;
        }
    }
    return *this;
}

Stringhelper &Stringhelper::replace(const std::string &before, const std::string &after, bool bCaseSensitive)
{
    if(bCaseSensitive)
    {
        replaceCaseSensitive(before,after);
    }
    else
    {
        replaceInCaseSensitive(before,after);
    }
    return *this;
}

int Stringhelper::compare(const std::string &s, bool bCaseSensitive) const
{
    if(bCaseSensitive)
    {
        if(_str<s)
        {
            return -1;
        }
        if(_str==s)
        {
            return 0;
        }
        return 1;
    }
    std::string l=tolower();
    std::string r=Stringhelper(s).tolower();
    if(l<r)
    {
        return -1;
    }
    if(l==r)
    {
        return 0;
    }
    return 1;

}

std::string Stringhelper::toupper() const
{
    std::string result;
    std::transform(_str.begin(),_str.end(),std::back_inserter(result),[](char c)->char{return std::toupper(c);});
    return result;
}

std::string Stringhelper::tolower() const
{
    std::string result;
    std::transform(_str.begin(),_str.end(),std::back_inserter(result),[](char c)->char{return std::tolower(c);});
    return result;
}
