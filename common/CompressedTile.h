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

#pragma once 

#include "MPI.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    /*! a plain, uncompressed tile */
    struct PlainTile 
    {
      PlainTile(const vec2i &tileSize, uint32_t *_pixel=NULL)
        : pitch(tileSize.x),
          pixel(_pixel?_pixel:new uint32_t [tileSize.x*tileSize.y]),
          myPixels(_pixel?false:true)
      {}
      ~PlainTile()
      { if (myPixels) delete[] pixel; }
      inline vec2i size() const { return region.size(); }
      /*! region of pixels that this tile corresponds to */
      box2i region;
      /*! number of ints in pixel[] buffer from one y to y+1 */
      int pitch;
      /*! which eye this goes to (if stereo) */
      int eye;
      /*! pointer to buffer of pixels; this buffer is 'pitch' int-sized pixels wide */
      uint32_t *pixel;
      /*! true if we allocated the pixels; false if not */
      bool myPixels;
    };

    /*! encoded representation of a tile - eventually to use true
        compression; for now we just pack all pixels (and header) into
        a single linear array of ints */
    struct CompressedTile {
      CompressedTile();
      ~CompressedTile();

      unsigned char *data;
      int fromRank;
      int numBytes;

      /*! get region that this tile corresponds to */
      box2i getRegion() const;

      /*! send the tile to the given rank in the given group */
      void sendTo(const MPI::Group &outside, const int targetRank) const;

      /*! receive one tile from the outside communicator */
      void receiveOne(const MPI::Group &outside); 
      void encode(void *compressor, const PlainTile &tile);
      void decode(void *decompressor, PlainTile &tile);

      static void *createCompressor();
      static void *createDecompressor();
      static void freeCompressor(void *);
      static void freeDecompressor(void *);
    };
    
  } // ::ospray::dw
} // ::ospray
