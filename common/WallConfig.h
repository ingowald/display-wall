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

#include "ospcommon/box.h"

// macro to turn on/off debugging printouts for the display wall
#define DW_DBG(a) /* nothing */

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
                 const vec2f &relativeBezelWidth=vec2f(0.f),
                 const DisplayArrangement displayArrangement=Arrangement_xy,
                 const bool stereo=0);

      /*! returns the number of pixels (in x and y, respectively) that
          the bezel will cover. Note we do not care whether bezels are
          symmetric in left/right respective in top/bottom direction:
          the 'x' value is the sum of left and right bezel area; the
          'y' value the sum of top and bottom area' */
      vec2i bezelPixelsPerDisplay() const;

      /*! computes the total number of pixels (in x and y directions,
          respectively), across all pixels, and INCLUDING "hidden"
          pixels in the bezels (ie, even though there is nothing to
          actually see in a bezel we report the bezel as if it was
          covered by pixels to avoid distortion) */
      vec2i  totalPixels()  const;
      /*! returns the total number of displays across x and y dimensions */
      size_t displayCount() const;
      /*! return total pixels in a frame, INCLUDING stereo (if enabled) */
      inline size_t totalPixelCount()   const { return (stereo?2:1)*totalPixels().x*totalPixels().y; }
      inline size_t displayPixelCount()   const { return (stereo?2:1)*pixelsPerDisplay.x*pixelsPerDisplay.y; }

      /*! return the rank (in the display proc group) of given display coordinates */
      int    rankOfDisplay(const vec2i &displayID) const;
      /*! return the X/Y display ID of the given display proc */
      vec2i  displayIDofRank(int rank) const;

      /*! return the pixel region in the global display wall space
        that display at given coordinates is covering */
      box2i  regionOfDisplay(const vec2i &displayID) const;
      box2i  regionOfRank(int rank) const;
      
      /*! returns range of displays that are affected by the given
          region of pixels (ie, that together are guaranteed to cover
          that pixel region */
      box2i  affectedDisplays(const box2i &pixelRegion) const;

      void   print() const;
      inline bool doStereo() const { return stereo; }

      vec2i numDisplays;
      vec2i pixelsPerDisplay;
      DisplayArrangement displayArrangement;
      bool stereo;
      vec2f relativeBezelWidth;
    };

  } // ::ospray::dw
} // ::ospray
