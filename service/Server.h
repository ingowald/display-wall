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

#include "../common/MPI.h"
#include "../common/WallConfig.h"
#include <thread>

namespace ospray {
  namespace dw {

    #define DW_STEREO 1
    #define DW_HAVE_HEAD_NODE 2
    
    typedef void (*DisplayCallback)(const uint32_t *leftEye, 
                                    const uint32_t *rightEye,
                                    void *objects);

    /*! the server that runs the display wall service (ie, the entity
        that communicates with the client(s), receives tiles, decodes
        them, and passes them to the display callback whenever a frame
        is ready */
    struct Server {
      Server(const MPI::Group &me,
             const WallConfig &wallConfig,
             const bool hasHeadNode,
             DisplayCallback displayCallback,
             void *objectForCallback,
             int desiredInfoPortNum);

      /*! the code that actually receives the tiles, decompresses
          them, and writes them into the current (write-)frame buffer */
      void processIncomingTiles(MPI::Group &outside);

      /*! note: this runs in its own thread */
      void setupCommunications(const WallConfig &wallConfig,
                               bool hasHeadNode,
                               const MPI::Group &world,
                               int desiredInfoPortNum);
      /*! open an MPI port and wait for the client(s) to connect to this
        port after this function terminates, all outward facing procs
        (either the head node, or all display nodes if no head node is
        being used) should have an proper MPI communicator set up to
        talk to the client rank(s) */
      MPI::Group waitForConnection(const MPI::Group &outwardFacingGroup,
                                   int desiredInfoPortNum);

      /*! allocate the frame buffers for left/right eye and recv/display, respectively */
      void allocateFrameBuffers();

      static Server *singleton;

      std::thread *commThread;
      /*! group that contails ALL display service procs, including the
          head node (if applicable) */
      const MPI::Group me;
      /*! group that contains only the display procs; either a
          intracomm (if a display node), or the intercomm (if head
          node) */
      MPI::Group displayGroup;

      const WallConfig wallConfig;
      const bool hasHeadNode;
      
      const DisplayCallback displayCallback;
      void *const objectForCallback;

      /*! total number of pixels already written this frame */
      size_t numWrittenThisFrame;
      /*! total number of pixels we have to write this frame until we
          have a full frame buffer */
      size_t numExpectedThisFrame;

      /*! @{ the four pixel arrays for left/right eye and
          receive/display buffers, respectively */
      uint32_t *recv_l, *recv_r, *disp_l, *disp_r;
      /*! @} */
    };

    void startDisplayWallService(const MPI_Comm comm,
                                 const WallConfig &wallConfig,
                                 bool hasHeadNode,
                                 DisplayCallback displayCallback,
                                 void *objectForCallback,
                                 int desiredInfoPortNum);

  } // ::ospray::dw
} // ::ospray
