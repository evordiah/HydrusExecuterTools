
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

#include "obs_nodeobject.h"
#include <sstream>
#include <fstream>
#include <QDir>
Obs_NodeObject::Obs_NodeObject(std::istream &in)
{
    using namespace std;
    int head[3];
    in.read((char*)(head),3*sizeof(int));
    _nFiledCnt=head[2]*3+1;
    const int linebytes=sizeof(float)*_nFiledCnt;
    if(head[1]!=linebytes)
    {
        cout<<"Obs_Node ERROR FORMAT"<<endl;
        return;
    }
    int nRec=head[0];
    _nLine=nRec;
    _data.reset(new float[nRec*_nFiledCnt]);

    int temp;
    for(int i=0;i<head[2];++i)
    {
        in.read((char*)&temp,sizeof(int));
        _nodes.push_back(temp);
    }

    char* pDt=(char*)_data.get();
    const int readRecordNum=100;
    int length=readRecordNum*linebytes;
    int nLine;
    while(in && nRec>0)
    {
        if(nRec<readRecordNum)
        {
            length=nRec*linebytes;
        }
        in.read(pDt,length);
        auto rsize=in.gcount();
        nLine=rsize/linebytes;
        pDt+=rsize;
        nRec-=nLine;
    }

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
}

Obs_NodeObject::Obs_NodeObject(const std::string &filename):Obs_NodeEncoder(filename)
{
    _nFiledCnt=_nodes.size()*3+1;
}

void Obs_NodeObject::SaveAsObs_NodeFile(const std::string &path)
{
    using namespace std;
    stringstream strbld;
    std::string head=" ******* Program HYDRUS\n"
                     " ******* \n"
                     " Welcome to HYDRUS-1D                                                    \n"
                     "[DATEANDTIME]\n"
                     " Units: L = [LUNIT]   , T = [TUNIT] , M = mmol \n";

    auto pos=head.find("[DATEANDTIME]");
    head=head.replace(pos,13,_DateandTime);
    pos=head.find("[LUNIT]");
    head=head.replace(pos,7,_LUnit);
    pos=head.find("[TUNIT]");
    head=head.replace(pos,7,_TUnit);
    strbld<<head;
    strbld<<"\n\n\n";

    strbld.width(40);
    strbld<<"Node("<<_nodes[0]<<")";
    for(unsigned int i=1;i<_nodes.size();++i)
    {
        strbld.width(30);
        strbld<<"Node("<<_nodes[i]<<")";
    }
    strbld<<std::endl;
    strbld<<"\n";

    strbld.width(13);
    strbld<<"time";
    for(unsigned int i=0;i<_nodes.size();++i)
    {
        strbld.width(12);
        strbld<<"h";
        strbld.width(13);
        strbld<<"theta";
        strbld.width(8);
        strbld<<"Flux";
    }
    strbld<<std::endl;
    strbld.setf(std::ios_base::showpoint);

    for(int i=0; i<_nLine; ++i)
    {
        strbld.width(16);
        strbld.precision(6);
        strbld<<Time(i);
        for(unsigned int j=0;j<_nodes.size();++j)
        {
            strbld.width(14);
            strbld.precision(6);
            strbld<<H(i,j);
            strbld.width(8);
            strbld.precision(4);
            strbld<<Theta(i,j);
            strbld.width(11);
            strbld.precision(3);
            strbld.setf(std::ios_base::scientific);
            strbld<<Flux(i,j);
            strbld.unsetf(std::ios_base::scientific);
        }
        strbld<<std::endl;
    }

    strbld<<"end"<<endl;
	QDir p(path.c_str());
	if (!p.exists())
	{
		p.mkpath(path.c_str());
	}
	std::string file = QDir::toNativeSeparators(p.absoluteFilePath("Obs_Node.out")).toStdString();
    std::ofstream out(file);
    out<<strbld.str();
    out.close();
}

std::istream& operator>>(std::istream &in, Obs_NodeObject &obj)
{
    obj.ParseFile(in);
    return in;
}

std::ostream& operator<<(std::ostream &out, const Obs_NodeObject &obj)
{
    out<<(const Obs_NodeEncoder&)(obj);
    return out;
}
