/* 
Copyright (c) 2016 Ingo Wald

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "../common/MPI.h"
#include "../common/WallConfig.h"
#include "../common/CompressedTile.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    struct Client {
      Client(const MPI::Group &me,
             const std::string &portName);
      void receiveDisplayConfig();
      /*! establish connection between 'me' and the remote service */
      void establishConnection(std::string portName);
      /*! return total pixels in display wall, so renderer/app can
          know how large a frame buffer to use ... */
      vec2i totalPixelsInWall() const;
      void writeTile(const PlainTile &tile);
      void endFrame();

      WallConfig *wallConfig;
      MPI::Group displayGroup;
      MPI::Group me;
    };

  } // ::ospray::dw
} // ::ospray
