#pragma once

#include "common/vec.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    typedef uint32_t uint32;

    struct WallConfig {
      WallConfig(const vec2i &numDisplays, const vec2i &pixelsPerDisplay)
        : numDisplays(numDisplays),
          pixelsPerDisplay(pixelsPerDisplay)
      {}

      vec2i numDisplays;
      vec2i pixelsPerDisplay;

      inline vec2i totalPixels()   const { return numDisplays * pixelsPerDisplay; }
      inline size_t displayCount() const { return numDisplays.x*numDisplays.y; }
      inline size_t pixelCount()   const { return totalPixels().x*totalPixels().y; }

      void print() const;
    };

    inline void WallConfig::print() const
    {
      std::cout << "WallConfig:" << std::endl;
      std::cout << " - num displays : " 
                << numDisplays.x << "x" << numDisplays.y
                << " (" << displayCount() << " displays)" << std::endl;
      std::cout << " - pixels/diplay: " 
                << pixelsPerDisplay.x << "x" << pixelsPerDisplay.y << std::endl;
      std::cout << " - total pixels : " 
                << totalPixels().x << "x" << totalPixels().y
                << " (" << prettyNumber(pixelCount()) << "pix)" << std::endl;
    }

  } // ::ospray::dw
} // ::ospray
