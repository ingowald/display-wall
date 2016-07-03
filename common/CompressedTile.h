#pragma once 

#include "common/box.h"
#include "common/MPI.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    /*! a plain, uncompressed tile */
    struct PlainTile 
    {
      /*! region of pixels that this tile corresponds to */
      box2i region;
      /*! guess... */
      int frameID;
      /*! number of ints in pixel[] buffer from one y to y+1 */
      int pitch;
      /*! pointer to buffer of pixels; this buffer is 'pitch' int-sized pixels wide */
      uint32_t *pixel;
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

      /*! send the tile to the given rank in the given group */
      void sendTo(const MPI::Group &outside, const int targetRank) const;

      /*! receive one tile from the outside communicator */
      void receiveOne(MPI::Group &outside, MPI::Group &me);
      void encode(const PlainTile &tile);
    };

    
  } // ::ospray::dw
} // ::ospray
