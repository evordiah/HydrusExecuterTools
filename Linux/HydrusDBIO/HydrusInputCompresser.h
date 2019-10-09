
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

#ifndef HYDRUSINPUTCOMPRESSER_H
#define HYDRUSINPUTCOMPRESSER_H
#include <string>
#include <memory>

class AtmosphObject;
class ProfileObject;
class SelectorObject;

class HydrusInputCompresser
{
   public:
    static bool Compress(const std::string& srcpath,const std::string& tofilename);
    static bool Compress(const SelectorObject& s,const AtmosphObject& a,const ProfileObject& p,const std::string& tofilename);
    static bool UnCompress(const std::string& filename,const std::string& topath);

    static std::unique_ptr<SelectorObject> ExtractSelector(const std::string& filename);
    static std::unique_ptr<AtmosphObject> ExtractAtmosph(const std::string& filename);
    static std::unique_ptr<ProfileObject> ExtractProfile(const std::string& filename);

    protected:

    private:
};

#endif // HYDRUSINPUTCOMPRESSER_H
