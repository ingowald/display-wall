#pragma once

#include "common/vec.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    struct WallConfig {
      WallConfig(const vec2i &numDisplays, const vec2i &pixelsPerDisplay);

      inline vec2i  totalPixels()  const { return numDisplays * pixelsPerDisplay; }
      inline size_t displayCount() const { return numDisplays.x*numDisplays.y; }
      inline size_t pixelCount()   const { return totalPixels().x*totalPixels().y; }
      
      int    rankOfDisplay(const vec2i &displayID) const;
      void print() const;

      vec2i numDisplays;
      vec2i pixelsPerDisplay;
    };

  } // ::ospray::dw
} // ::ospray
