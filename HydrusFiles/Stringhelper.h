#include <utility>

#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <string>
#include <vector>

class Stringhelper
{
public:
    Stringhelper(const std::string&  s):_str(s) {}
    Stringhelper(const char* c):_str(c){}
    Stringhelper(const Stringhelper& rhs)
    {
        if(this!=&rhs)
        {
            _str=rhs._str;
        }
    }
    Stringhelper& operator=(const Stringhelper& rhs)
    {
        if(this!=&rhs)
        {
            _str=rhs._str;
        }
        return *this;
    }
    std::string str() const
    {
        return _str;
    }
    Stringhelper& arg(long long a);
    Stringhelper& arg(unsigned long long a);
    Stringhelper& arg(long a);
    Stringhelper& arg(unsigned long a);
    Stringhelper& arg(int a);
    Stringhelper& arg(unsigned int a);
    Stringhelper& arg(short a);
    Stringhelper& arg(unsigned short a);
    Stringhelper& arg(double a);
    Stringhelper& arg(char a);
    Stringhelper& arg(unsigned char a);
    Stringhelper& arg(const std::string &a);
    Stringhelper& simplified();
    Stringhelper& trimmed();
    std::vector<std::string> split(const std::string &sep) const;
    std::vector<std::string> split( char sep) const;
    bool startsWith(const std::string& s,bool bcaseSensitive=true) const;
    bool startsWith( char c,bool bcaseSensitive=true) const;
    Stringhelper& replace( char before,  char after, bool bCaseSensitive=true);
    Stringhelper& replace(const std::string& before, const std::string& after, bool bCaseSensitive=true);
    int compare(const std::string& s,bool bCaseSensitive=true) const;
    std::string toupper() const;
    std::string tolower() const;
protected:
    std::pair<size_t,size_t> findArgEscapes();
    void replaceArgEscapes(const std::string& s);
    void replaceCaseSensitive(const std::string& before, const std::string& after);
    void replaceInCaseSensitive(const std::string& before, const std::string& after);
private:
    std::string _str;
};

#endif // STRINGHELPER_H
