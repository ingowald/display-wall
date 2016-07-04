
#include "Server.h"
#include "../common/CompressedTile.h"

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

#if 0
    struct TileWriter {
      TileWriter(GlutWindow *window);

      FrameBuffer *writeFB, *displayFB;
      GlutWindow  *window;
      size_t       numPixelsWrittenThisFrame;
      size_t       totalPixelsInFrame;
      std::mutex   mutex;
    };
#endif

    /*! the code that actually receives the tiles, decompresses
      them, and writes them into the current (write-)frame buffer */
    void Server::processIncomingTiles(MPI::Group &outside)
    {
      allocateFrameBuffers();
      printf("tile receiver %i/%i: frame buffer(s) allocated; now receiving tiles\n",
             displayGroup.rank,displayGroup.size);
      
      const box2i displayRegion = wallConfig.regionOfRank(displayGroup.rank);
      PRINT(displayRegion);

      if (wallConfig.stereo)
        // for doing stereo, we have to somehow pass left/right eye
        // info with the tile; this isn't done yet
        throw std::runtime_error("stereo not implemented yet");

      while (1) {
        // -------------------------------------------------------
        // receive one tiles
        // -------------------------------------------------------
        CompressedTile encoded;
        encoded.receiveOne(outside);

        PlainTile plain(encoded.getRegion().size());
        encoded.decode(plain);

        const box2i globalRegion = plain.region;
        size_t numWritten = 0;
        const uint32_t *tilePixel = plain.pixel;
        uint32_t *localPixel = recv_l;
        for (int iy=globalRegion.lower.y;iy<globalRegion.upper.y;iy++) {

          if (iy < displayRegion.lower.y) continue;
          if (iy >= displayRegion.upper.y) continue;
          
          for (int ix=globalRegion.lower.x;ix<globalRegion.upper.x;ix++) {
            if (ix < displayRegion.lower.x) continue;
            if (ix >= displayRegion.upper.x) continue;

            const vec2i globalCoord(ix,iy);
            const vec2i tileCoord = globalCoord-plain.region.lower;
            const vec2i localCoord = globalCoord-displayRegion.lower;
            const int localPitch = wallConfig.pixelsPerDisplay.x;
            const int tilePitch  = plain.pitch;
            const int tileOfs = tileCoord.x + tilePitch * tileCoord.y;
            const int localOfs = localCoord.x + localPitch * localCoord.y;
            localPixel[localOfs] = tilePixel[tileOfs];
            ++numWritten;
          }
        }
        numWrittenThisFrame += numWritten;
        if (numWrittenThisFrame == numExpectedThisFrame) {
          printf("display %i/%i has a full frame!\n",displayGroup.rank,displayGroup.size);

          displayGroup.barrier();
          printf("display %i/%i DISPLAYING\n",displayGroup.rank,displayGroup.size);

          displayCallback(recv_l,recv_r,objectForCallback);
          // reset counter
          numWrittenThisFrame = 0;
          numExpectedThisFrame = wallConfig.pixelCount();
          // and switch the in/out buffers
          std::swap(recv_l,disp_l);
          std::swap(recv_r,disp_r);
        }
      }
    }

  } // ::ospray::dw
} // ::ospray
