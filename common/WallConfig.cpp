#pragma once

#include "WallConfig.h"

namespace ospray {
  namespace dw {

    WallConfig::WallConfig(const vec2i &numDisplays, const vec2i &pixelsPerDisplay)
      : numDisplays(numDisplays),
        pixelsPerDisplay(pixelsPerDisplay)
    {}
    
    int    WallConfig::rankOfDisplay(const vec2i &displayID) const 
    {
      return displayID.x+numDisplays.x*displayID.y;
    }
    
    void WallConfig::print() const
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
