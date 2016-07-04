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

#pragma once

#include "common/vec.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    struct WallConfig {
      /*! how the displays are arranged - the first letter describes
          the first axis of enumeration ('x' from left to right, 'X'
          from right to left, 'y' from bottom to top, 'Y' from top to
          bottom); the second letter is the second axis. Ie, 'xy' (on
          a 5x3 walld means rank 0 (ie, display (0,0)) is in the lower
          left, rank 1 (ie, (1,0)) to the right of it, rank 5 (display
          (0,1)) to the top of 0, etc.  */
      typedef enum { Arrangement_xy, Arrangement_xY, Arrangement_Xy, Arrangement_XY, 
                     Arrangement_yx, Arrangement_yX, Arrangement_Yx, Arrangement_YX } 
        DisplayArrangement;

      WallConfig(const vec2i &numDisplays, 
                 const vec2i &pixelsPerDisplay,
                 const DisplayArrangement displayArrangement=Arrangement_xy,
                 const bool stereo=0);

      inline vec2i  totalPixels()  const { return numDisplays * pixelsPerDisplay; }
      inline size_t displayCount() const { return numDisplays.x*numDisplays.y; }
      inline size_t pixelCount()   const { return totalPixels().x*totalPixels().y; }
      
      int    rankOfDisplay(const vec2i &displayID) const;
      void   print() const;

      vec2i numDisplays;
      vec2i pixelsPerDisplay;
      DisplayArrangement displayArrangement;
      bool stereo;
    };

  } // ::ospray::dw
} // ::ospray
