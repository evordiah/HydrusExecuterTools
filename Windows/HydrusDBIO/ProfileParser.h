
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

#ifndef PROFILEPARSER_H
#define PROFILEPARSER_H
#include <string>
#include <iostream>
#include <memory>

class ProfileEncoder
{
    public:
        ProfileEncoder(const std::string& filename);
        virtual ~ProfileEncoder(){}
        //return the whole size of file,including head
        unsigned int GetLength() const
        {
            int cnt=2*sizeof(int)+sizeof(float);
            cnt+=_nodecnt*sizeof(int)*2;
            cnt+=_nodecnt*sizeof(float)*6;
            cnt+=_observercnt*sizeof(int);
            return cnt;
        }
        bool ParseFile(const std::string& filename);
        friend std::ostream& operator<<(std::ostream& out,const ProfileEncoder& ProEncoder);
    protected:
        ProfileEncoder();
        void AllocateMemory();
        float _depth;
        int _nodecnt;
        int _observercnt;
        std::unique_ptr<float[]> _xcoord;
        std::unique_ptr<float[]> _h;
        std::unique_ptr<int[]> _mat;
        std::unique_ptr<int[]> _lay;
        std::unique_ptr<float[]> _beta;
        std::unique_ptr<float[]> _Ah;
        std::unique_ptr<float[]> _Ak;
        std::unique_ptr<float[]> _Ath;
        std::unique_ptr<int[]> _observeNodeid;
        bool ParseFile(std::istream& in);
private:
        bool ParseBlockH(std::istream& in);

};


#endif // PROFILEPARSER_H
