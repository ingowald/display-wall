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
      // if (displayArrangement != Arrangement_xy)
      //   throw std::runtime_error("non-default arrangments of displays not yet implemented");
    }
    
    
    vec2i  WallConfig::displayIDofRank(int rank) const
    {
      switch (displayArrangement) {
      case Arrangement_Yx:
        return vec2i(rank / numDisplays.y,
                     numDisplays.y-1-(rank % numDisplays.y));
      case Arrangement_xy:
        return vec2i(rank % numDisplays.x,rank / numDisplays.x);
      case Arrangement_xY:
        return vec2i(rank % numDisplays.x,numDisplays.y-1-(rank / numDisplays.x));
      default:
        throw std::runtime_error("display arrangement not implemented ...");
      }
    }

    int    WallConfig::rankOfDisplay(const vec2i &displayID) const 
    {
      switch (displayArrangement) {
      case Arrangement_Yx:
        return (numDisplays.y-1-displayID.y)+numDisplays.y*displayID.x;
      case Arrangement_xy:
        return displayID.x+numDisplays.x*displayID.y;
      case Arrangement_xY:
        return displayID.x+numDisplays.x*(numDisplays.y-1-displayID.y);
      default:
        throw std::runtime_error("display arrangement not implemented ...");
      }
    }
    

    box2i  WallConfig::regionOfRank(int rank) const
    { 
      return regionOfDisplay(displayIDofRank(rank)); 
    }



    /*! returns range of displays that are affected by the given
      region of pixels (ie, that together are guaranteed to cover
      that pixel region */
    box2i  WallConfig::affectedDisplays(const box2i &pixelRegion) const
    {
#if 1
      vec2i lo, hi;
      for (int ix=0;ix<numDisplays.x;ix++) {
        int ix_begin = ix*(pixelsPerDisplay.x+bezelPixelsPerDisplay().x);
        int ix_end   = ix_begin+pixelsPerDisplay.x+bezelPixelsPerDisplay().x;
        
        if (ix_end > pixelRegion.lower.x) { lo.x = ix; break; }
      }
      for (int ix=numDisplays.x-1;ix>=0;--ix) {
        int ix_begin = ix*(pixelsPerDisplay.x+bezelPixelsPerDisplay().x);
        int ix_end   = ix_begin+pixelsPerDisplay.x+bezelPixelsPerDisplay().x;
        
        if (ix_begin < pixelRegion.upper.x) { hi.x = ix+1; break; }
      }
      for (int iy=0;iy<numDisplays.y;iy++) {
        int iy_begin = iy*(pixelsPerDisplay.y+bezelPixelsPerDisplay().y);
        int iy_end   = iy_begin+pixelsPerDisplay.y+bezelPixelsPerDisplay().y;
        
        if (iy_end > pixelRegion.lower.y) { lo.y = iy; break; }
      }
      for (int iy=numDisplays.y-1;iy>=0;--iy) {
        int iy_begin = iy*(pixelsPerDisplay.y+bezelPixelsPerDisplay().y);
        int iy_end   = iy_begin+pixelsPerDisplay.y+bezelPixelsPerDisplay().y;
        
        if (iy_begin < pixelRegion.upper.y) { hi.y = iy+1; break; }
      }
      
      box2i result(lo,hi);
      
      return result;
#else
      vec2i lo
        = (pixelRegion.lower+bezelPixelsPerDisplay())
        / (pixelsPerDisplay+bezelPixelsPerDisplay());
      vec2i hi
        = (pixelRegion.upper + pixelsPerDisplay - vec2i(1)) 
        / (pixelsPerDisplay+bezelPixelsPerDisplay());
      // vec2i hi
      // = divRoundUp(pixelRegion.upper,pixelsPerDisplay+bezelPixelsPerDisplay());
      if (hi.x > numDisplays.x || hi.y > numDisplays.y) {
        throw std::runtime_error("invalid region in 'affectedDispalys()')");
      }
      return box2i(lo,hi);
#endif
    }


    /*! return the pixel region in the global display wall space that
        display at given coordinates is covering */
    box2i  WallConfig::regionOfDisplay(const vec2i &displayID) const
    {
      const vec2i lo = displayID * (pixelsPerDisplay + bezelPixelsPerDisplay()); 
      return box2i(lo, lo + pixelsPerDisplay);
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
