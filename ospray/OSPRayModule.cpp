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

// ospray pixel 
#include "ospray/fb/PixelOp.h"
// displaywald client
#include "../client/Client.h"

namespace ospray {
  namespace dw {

    void foo()
    {
    }

    struct DisplayWaldPixelOp : public ospray::PixelOp 
    {
      // /*! gets called every time the frame buffer got 'commit'ted */
      // virtual void  commitNotify() {}
      // /*! gets called once at the beginning of the frame */
      // virtual void beginFrame() {}
      // /*! gets called once at the end of the frame */
      virtual void endFrame() {}
      
      // /*! called whenever a new tile comes in from a renderer, but
      //     _before_ the tile gets written/accumulated into the frame
      //     buffer. this way we can, for example, fill in missing
      //     samples; however, the tile will _not_ yet contain the
      //     previous frame's contributions from the accum buffer
      //     etcpp. In distriubuted mode, it is undefined if this op gets
      //     executed on the node that _produces_ the tile, or on the
      //     node that _owns_ the tile (and its accum buffer data)  */
      // virtual void preAccum(Tile &tile) {}

      /*! called right after the tile got accumulated; i.e., the
          tile's RGBA values already contain the accu-buffer blended
          values (assuming an accubuffer exists), and this function
          defines how these pixels are being processed before written
          into the color buffer */
      virtual void postAccum(Tile &tile) {}

      //! \brief common function to help printf-debugging 
      /*! Every derived class should overrride this! */
      virtual std::string toString() const;
    };

    //! \brief common function to help printf-debugging 
    std::string DisplayWaldPixelOp::toString() const 
    { return "ospray::dw::DisplayWaldPixelOp (displayWald module)"; }

    extern "C" void ospray_init_module_displayWald()
    {
      printf("loading the 'displayWald' module...\n");
    }

    OSP_REGISTER_PIXEL_OP(DisplayWaldPixelOp,display_wald);

  } // ::ospray::dw
} // ::ospray
