
#include "Server.h"
#include "../common/CompressedTile.h"
#include "ospcommon/tasking/parallel_for.h"
#include <mutex>
#ifdef OSPRAY_TASKING_TBB
# include <tbb/task_scheduler_init.h>
#endif

namespace ospray {
  namespace dw {

#define DW_DBG(a) 

    using std::cout; 
    using std::endl;
    using std::flush;

    /*! the code that actually receives the tiles, decompresses
      them, and writes them into the current (write-)frame buffer */
    void Server::processIncomingTiles(MPI::Group &outside)
    {
      allocateFrameBuffers();
//       printf("tile receiver %i/%i: frame buffer(s) allocated; now receiving tiles\n",
//              displayGroup.rank,displayGroup.size);
      
      const box2i displayRegion = wallConfig.regionOfRank(displayGroup.rank);

#define THREADED_RECV 3
        
#if THREADED_RECV
      std::mutex displayMutex;
# ifdef OSPRAY_TASKING_TBB
      tbb::task_scheduler_init tbb_init;
# endif
      parallel_for(THREADED_RECV,[&](int) {
#endif
          void *decompressor = CompressedTile::createDecompressor();
          while (1) {
            // -------------------------------------------------------
            // receive one tiles
            // -------------------------------------------------------

            CompressedTile encoded;
            encoded.receiveOne(outside);

            PlainTile plain(encoded.getRegion().size());
            encoded.decode(decompressor,plain);

            const box2i globalRegion = plain.region;
            size_t numWritten = 0;
            const uint32_t *tilePixel = plain.pixel;
            uint32_t *localPixel = plain.eye ? recv_r : recv_l;
            assert(localPixel);
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

            {
#if THREADED_RECV
              std::lock_guard<std::mutex> lock(displayMutex);
#endif
              numWrittenThisFrame += numWritten;
              // printf("written %li / %li\n",numWrittenThisFrame,numExpectedThisFrame);
              if (numWrittenThisFrame == numExpectedThisFrame) {
                DW_DBG(printf("display %i/%i has a full frame!\n",
                              displayGroup.rank,displayGroup.size));
          
                // displayGroup.barrier();
                DW_DBG(printf("#osp:dw(%i/%i) barrier'ing on %i/%i\n",
                              displayGroup.rank,displayGroup.size,
                              outside.rank,outside.size));
                MPI_CALL(Barrier(outside.comm));
                DW_DBG(printf("#osp:dw(%i/%i): DISPLAYING\n",
                              displayGroup.rank,displayGroup.size));
          
                displayCallback(recv_l,recv_r,objectForCallback);
                // reset counter
                numWrittenThisFrame = 0;
                numExpectedThisFrame = wallConfig.displayPixelCount();
                // and switch the in/out buffers
                std::swap(recv_l,disp_l);
                std::swap(recv_r,disp_r);
              }
            }
          }
          CompressedTile::freeDecompressor(decompressor);
#if THREADED_RECV
        });
#endif
    }

  } // ::ospray::dw
} // ::ospray
