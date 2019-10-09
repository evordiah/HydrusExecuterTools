
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

#ifndef ATMOSPHPARSER_H
#define ATMOSPHPARSER_H
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <vector>

class AtmosphEncoder
{
    public:
        AtmosphEncoder(const std::string& filename);
        virtual ~AtmosphEncoder(){}
        //return the whole size of file,including head
        unsigned int GetLength() const
        {
            int cnt=sizeof(int)+sizeof(float);
            cnt+=5*sizeof(float)*_maxAL;
            return cnt;
        }
        bool ParseFile(const std::string& filename);
        friend std::ostream& operator<<(std::ostream& out,const AtmosphEncoder& AtmEncoder);
    protected:
        AtmosphEncoder();
        int _maxAL;
        float _hCrits;
        std::unique_ptr<float[]> _data;
        std::map<std::string,bool> _logicoptions;
        bool ParseFile(std::istream& in);
        std::vector<bool> GetLogicalOptions(const std::string &val);
private:
        bool ParseBlockI(std::istream& in);

};


#endif // ATMOSPHPARSER_H
