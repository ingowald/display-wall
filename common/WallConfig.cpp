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
    
    vec2i  WallConfig::displayIDofRank(int rank) const
    {
      return vec2i(rank % numDisplays.x,rank / numDisplays.x);
    }

    box2i  WallConfig::regionOfDisplay(const vec2i &displayID) const
    {
      return box2i(displayID * pixelsPerDisplay,
                   displayID * pixelsPerDisplay + pixelsPerDisplay);
    }

    box2i  WallConfig::regionOfRank(int rank) const
    { 
      return regionOfDisplay(displayIDofRank(rank)); 
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
                << " (" << prettyNumber(totalPixelCount()) << "pix)" << std::endl;
    }

  } // ::ospray::dw
} // ::ospray
