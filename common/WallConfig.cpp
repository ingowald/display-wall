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
                           const vec2f &relativeBezelWidth,
                           const DisplayArrangement displayArrangement,
                           const bool stereo)
      : numDisplays(numDisplays),
        pixelsPerDisplay(pixelsPerDisplay),
        relativeBezelWidth(relativeBezelWidth),
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


    /*! returns range of displays that are affected by the given
      region of pixels (ie, that together are guaranteed to cover
      that pixel region */
    box2i  WallConfig::affectedDisplays(const box2i &pixelRegion) const
    {
      vec2i lo
        = (pixelRegion.lower+bezelPixelsPerDisplay())
        / (pixelsPerDisplay+bezelPixelsPerDisplay());
      vec2i hi
        = (pixelRegion.upper + pixelsPerDisplay - vec2i(1)) 
        / (pixelsPerDisplay+bezelPixelsPerDisplay());
      // vec2i hi
      // = divRoundUp(pixelRegion.upper,pixelsPerDisplay+bezelPixelsPerDisplay());
      if (hi.x > numDisplays.x || hi.y > numDisplays.y) 
        throw std::runtime_error("invalid region in 'affectedDispalys()')");
      return box2i(lo,hi);
    }


    /*! return the pixel region in the global display wall space that
        display at given coordinates is covering */
    box2i  WallConfig::regionOfDisplay(const vec2i &displayID) const
    {
      const vec2i lo = displayID * (pixelsPerDisplay + bezelPixelsPerDisplay()); 
      return box2i(lo, lo + pixelsPerDisplay);
    }

    box2i  WallConfig::regionOfRank(int rank) const
    { 
      return regionOfDisplay(displayIDofRank(rank)); 
    }

    int    WallConfig::rankOfDisplay(const vec2i &displayID) const 
    {
      return displayID.x+numDisplays.x*displayID.y;
    }
    
    /*! returns the total number of displays across x and y dimensions */
    size_t WallConfig::displayCount() const
    { 
      return numDisplays.x*numDisplays.y; 
    }

    /*! returns the number of pixels (in x and y, respectively) that
      the bezel will cover. Note we do not care whether bezels are
      symmetric in left/right respective in top/bottom direction:
      the 'x' value is the sum of left and right bezel area; the
      'y' value the sum of top and bottom area' */
    vec2i WallConfig::bezelPixelsPerDisplay() const
    {
      return vec2i(relativeBezelWidth*vec2f(pixelsPerDisplay)); 
    }
    
    /*! computes the total number of pixels (in x and y directions,
      respectively), across all pixels, and INCLUDING "hidden"
      pixels in the bezels (ie, even though there is nothing to
      actually see in a bezel we report the bezel as if it was
      covered by pixels to avoid distortion) */
    vec2i WallConfig::totalPixels() const
    { 
      const vec2i realPixels = numDisplays * pixelsPerDisplay;
      const vec2i bezelPixels = (numDisplays-vec2i(1)) * bezelPixelsPerDisplay();
      return realPixels+bezelPixels; 
    }

    void WallConfig::print() const
    {
      std::cout << "WallConfig:" << std::endl;
      std::cout << " - num displays : " 
                << numDisplays.x << "x" << numDisplays.y
                << " (" << displayCount() << " displays)" << std::endl;
      std::cout << " - pixels/diplay: " 
                << pixelsPerDisplay.x << "x" << pixelsPerDisplay.y << std::endl;
      std::cout << " - bezel width "
                << "x:" << int(100.f*relativeBezelWidth.x) << "%, "
                << "y:" << int(100.f*relativeBezelWidth.y) << "%";
      std::cout << " ("
                << "x:" << int(pixelsPerDisplay.x*relativeBezelWidth.x) << "pix, "
                << "y:" << int(pixelsPerDisplay.y*relativeBezelWidth.y) << "pix)" 
                << std::endl;
      std::cout << " - total pixels : " 
                << totalPixels().x << "x" << totalPixels().y
                << " (" << prettyNumber(totalPixelCount()) << "pix)" << std::endl;
    }

  } // ::ospray::dw
} // ::ospray
