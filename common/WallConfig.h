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
