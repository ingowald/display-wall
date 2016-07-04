#include "WallConfig.h"

namespace ospray {
  namespace dw {

    WallConfig::WallConfig(const vec2i &numDisplays, 
                           const vec2i &pixelsPerDisplay,
                           const DisplayArrangement displayArrangement,
                           const bool stereo)
      : numDisplays(numDisplays),
        pixelsPerDisplay(pixelsPerDisplay),
        displayArrangement(displayArrangement),
        stereo(stereo)
    {
      if (displayArrangement != Arrangement_xy)
        throw std::runtime_error("non-default arrangments of displays not yet implemented");
    }
    
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
