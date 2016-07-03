#pragma once

// ospray
#include "ospray/ospray.h"
#include "common/box.h"

namespace ospray {
  using namespace ospcommon;

  namespace dw {
    typedef uint32_t uint32;

    /*! a (stereo-capable) frame buffer */
    struct FrameBuffer {
      FrameBuffer(const vec2i &size, bool stereo=false)
        : size(size)
      {
        const size_t numPixels = size.x*size.y;
        pixels[0] = new uint32[numPixels];
        if (stereo)
          pixels[1] = new uint32[numPixels];
        else 
          pixels[1] = NULL;
      }

      /*! number of pixels (per eye, if in stereo mode */
      vec2i   size;

      /* the (up to) two frame buffers. without stero this'll be jsut
         a single frame buffer in pixels[0] (pixels[1] will be NULL);
         otherwise we have two frame buffer, with left eye in
         pixels[0] and right eye in pixels[1] */
      uint32 *pixels[2];
    };

  } // ::ospray::dw
} // ::ospray
