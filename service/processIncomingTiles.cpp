// ****************************************************************************** //
// Copyright (c) 2016-2017 Ingo Wald                                              //
//                                                                                //
// Permission is hereby granted, free of charge, to any person obtaining a copy   //
// of this software and associated documentation files (the "Software"), to deal  //
// in the Software without restriction, including without limitation the rights   //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      //
// copies of the Software, and to permit persons to whom the Software is          //
// furnished to do so, subject to the following conditions:                       //
//                                                                                //
// The above copyright notice and this permission notice shall be included in all //
// copies or substantial portions of the Software.                                //
//                                                                                //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  //
// SOFTWARE.                                                                      //
// ****************************************************************************** //

// ours
#include "Server.h"
#include "common/CompressedTile.h"
// ospcommon
#include "ospcommon/tasking/parallel_for.h"
// std
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
      printf("tile receiver %i/%i: frame buffer(s) allocated; now receiving tiles\n",
             displayGroup.rank,displayGroup.size);
      
      const box2i displayRegion = wallConfig.regionOfRank(displayGroup.rank);
#define THREADED_RECV 8
        
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
