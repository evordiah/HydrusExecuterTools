
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

#include "BalanceObject.h"
#include <sstream>
#include <QDir>
#include <fstream>

BalanceObject::BalanceObject(std::istream& in)
{
    char regionnum,timenum;
    in.read(&regionnum,1);
    _regionnum=regionnum;
    in.read(&timenum,1);
    _timenum=timenum;
    float temp;
    for(int i=0; i<(int)timenum; ++i)
    {
        in.read((char*)&temp,sizeof(float));
        vec_time.push_back(temp);
    }
    unsigned int ncount=0;
    short nnum;
    for(int i=0; i<(int)timenum; ++i)
    {
        in.read((char*)&nnum,sizeof(short));
        vec_num.push_back(nnum);
        ncount+=nnum;
    }
    std::unique_ptr<float[]> pdata(new float[ncount]);
    in.read((char*)pdata.get(),ncount*sizeof(float));
    for(unsigned int i=0;i<ncount;++i)
    {
        vec_data.push_back(pdata[i]);
    }
    _headnum=4*(_regionnum+1)+2;
    _othernum=_headnum+2;

    int s;
    char desinfo[100];
    in.read((char*)(&s),sizeof(int));
    in.read(desinfo,s);
    desinfo[s]=0;
    _DateandTime=desinfo;
    in.read((char*)(&s),sizeof(int));
    in.read(desinfo,s);
    desinfo[s]=0;
    _LUnit=desinfo;
    in.read((char*)(&s),sizeof(int));
    in.read(desinfo,s);
    desinfo[s]=0;
    _TUnit=desinfo;
    in.read((char*)(&_caltime),sizeof(double));
}

BalanceObject::BalanceObject(const std::string &filename):BalanceEncoder(filename)
{
}

void BalanceObject::SaveAsBalancefFile(const std::string &path)
{
	QDir p(path.c_str());
	if (!p.exists())
	{
		if (!p.mkpath(path.c_str()))
		{
			return;
		}
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("Balance.out")).toStdString();
    Initial();
    std::ofstream out(file);
    out<<FormatFile();
    out.close();
}

std::istream& operator>>(std::istream &in, BalanceObject &obj)
{
    obj.ParseFile(in);
    return in;
}

std::ostream& operator<<(std::ostream &out, const BalanceObject &obj)
{
    out<<(BalanceEncoder&)obj;
    return out;
}

void BalanceObject::Initial()
{
    using namespace std;
    vec_lineheads= {" Length   [L]      "," W-volume [L]      ",
                    " In-flow  [L/T]    "," h Mean   [L]      ",
                    " Top Flux [L/T]    "," Bot Flux [L/T]    ",
                    " WatBalT  [L]      "," WatBalR  [%]      "
                   };
    stringstream strbld;
    vector<string> values;
    int cnt=vec_num[0];
    int cntperline=(cnt-2)/4;
    int pos=0;
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<cntperline; j++)
        {
            strbld<<FormatFloat(vec_data[pos++]);
        }
        strbld<<endl;
        values.push_back(strbld.str());
        strbld.str("");
    }
    for(int i=0; i<2; i++)
    {
        strbld<<FormatFloat(vec_data[pos++])<<endl;
        values.push_back(strbld.str());
        strbld.str("");
    }
    vec_linecontents.push_back(values);
    for(unsigned int k=1; k<vec_num.size(); k++)
    {
        values.clear();
        cnt=vec_num[k];
        cntperline=(cnt-4)/4;
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<cntperline; j++)
            {
                strbld<<FormatFloat(vec_data[pos++]);
            }
            strbld<<endl;
            values.push_back(strbld.str());
            strbld.str("");
        }
        for(int i=0; i<3; i++)
        {
            strbld<<FormatFloat(vec_data[pos++])<<endl;
            values.push_back(strbld.str());
            strbld.str("");
        }
        strbld.width(13);
        strbld.precision(3);
        strbld<<fixed;
        strbld<<vec_data[pos++]<<endl;
        values.push_back(strbld.str());
        strbld.str("");
        vec_linecontents.push_back(values);
    }
}

std::string BalanceObject::FormatFloat(float value)
{
    std::stringstream strbld;
    strbld<<std::scientific;
    strbld.precision(4);
    strbld<<std::uppercase;
    strbld<<value*10;
    std::string svalue=strbld.str();
    strbld.str("");
    float fv;
    std::string postfix;
    if(svalue.size()==10)
    {
        fv=stof(svalue.substr(0,6))/10;
        postfix=svalue.substr(6);
    }
    else
    {
        fv=stof(svalue.substr(0,7))/10;
        postfix=svalue.substr(7);
    }
    strbld.width(9);
    strbld<<std::fixed;
    strbld.precision(5);
    strbld<<fv<<postfix;
    return strbld.str();
}

std::string BalanceObject::FormatLine(int timeindex,int lineindex)
{
    using namespace std;
    stringstream strbld;
    switch(lineindex)
    {
    case 1:
    case 3:
    case 5:
        return "----------------------------------------------------------\n";
    case 2:
        strbld<<" Time       [T]";
        strbld.width(14);
        strbld<<fixed;
        strbld.precision(4);
        strbld<<vec_time[timeindex]<<endl;
        return strbld.str();
    case 4:
        strbld<<" Sub-region num.";
        strbld.width(22);
        strbld<<1;
        for(int i=2; i<=_regionnum; ++i)
        {
            strbld.width(13);
            strbld<<i;
        }
        strbld<<std::endl;
        return strbld.str();
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        strbld<<vec_lineheads[lineindex-6]<<vec_linecontents[timeindex][lineindex-6];
        return strbld.str();
    case 12:
        if(timeindex==0)
        {
            return "----------------------------------------------------------\n";
        }
        else
        {
            strbld<<vec_lineheads[lineindex-6]<<vec_linecontents[timeindex][lineindex-6];
            return strbld.str();
        }
    case 13:
        if(timeindex==0)
        {
            return "\n";
        }
        else
        {
            strbld<<vec_lineheads[lineindex-6]<<vec_linecontents[timeindex][lineindex-6];
            return strbld.str();
        }
    case 14:
        return "----------------------------------------------------------\n";
    case 15:
        return " \n";
    }
    return "";
}

std::string BalanceObject::FormatFile()
{
    using namespace std;
    stringstream strbld;
    string head=" ******* Program HYDRUS\n"
                " ******* \n"
                " Welcome to HYDRUS-1D                                                    \n"
                "[DATEANDTIME]\n"
                " Units: L = [LUNIT]   , T = [TUNIT] , M = mmol \n"
                "\n";
    auto pos=head.find("[DATEANDTIME]");
    head=head.replace(pos,13,_DateandTime);
    pos=head.find("[LUNIT]");
    head=head.replace(pos,7,_LUnit);
    pos=head.find("[TUNIT]");
    head=head.replace(pos,7,_TUnit);
    strbld<<head;
    for(int i=1; i<14; i++)
    {
        strbld<<FormatLine(0,i);
    }
    for(unsigned int j=1; j<vec_time.size(); j++)
    {
        for(int i=1; i<16; i++)
        {
            strbld<<FormatLine(j,i);
        }
    }
    std::string tail=" Calculation time [sec]   [CALTIME]     ";
    pos=tail.find("[CALTIME]");
    tail=tail.replace(pos,9,std::to_string(_caltime));
    strbld<<tail<<endl;
    return strbld.str();
}

