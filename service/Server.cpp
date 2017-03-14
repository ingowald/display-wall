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

#include "Server.h"
#include "../common/CompressedTile.h"
#include "ospcommon/Socket.h"

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    Server *Server::singleton = NULL;

    std::mutex commThreadIsReady;
    std::mutex canStartProcessing;

    std::thread Server::commThread;

    /*! create a port at a well-defined port ID, and use this to serve
        - via a simple TCP/IP port - the name of the MPI port, the
        parameters of the display wall, etc. Note this (should) be
        only called by outward facing rank 0 */
    void openInfoPort(const std::string &mpiPortName,
                      const WallConfig &wallConfig,
                      int desiredInfoPortNum)
    {
      socket_t listener = NULL;
      int nextPortToTry = desiredInfoPortNum;

      while (listener == NULL) {
        listener = bind(nextPortToTry);
        if (listener != NULL)
          break;
        nextPortToTry++;
      }
      
      char hostName[1000];
      gethostname(hostName,1000);
      printf("=======================================================\n");
      printf("#osp:dw: opened display wall info port on %s:%i\n",
             hostName,nextPortToTry);
      printf("=======================================================\n");
      
      static std::thread portListenerThread([=](){
          while (1) {
            socket_t client = listen(listener);
            printf("#osp:dw: got client connection...\n");
            write(client,mpiPortName);
            write(client,wallConfig.totalPixels().x);
            write(client,wallConfig.totalPixels().y);
            write(client,(int)wallConfig.stereo);
            flush(client);
            close(client);
          }
        });
    }

    /*! send the display wall config to the client, so the client will
        known both display arrayngement and total frame buffer
        config */
    void sendConfigToClient(const MPI::Group &outside, 
                            const MPI::Group &me,
                            const WallConfig &wallConfig)
    {
      vec2i numDisplays = wallConfig.numDisplays;
      vec2i pixelsPerDisplay = wallConfig.pixelsPerDisplay;
      vec2f relativeBezelWidth = wallConfig.relativeBezelWidth;
      int arrangement = wallConfig.displayArrangement;
      int stereo      = wallConfig.stereo;
      /*! if we're the head node, let's 'fake' a single display to the client */
      if (me.size == 1) {
        pixelsPerDisplay = wallConfig.totalPixels();
        numDisplays = vec2i(1);
        relativeBezelWidth = vec2f(0.f);
      }
      MPI_CALL(Bcast(&numDisplays,2,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,2,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&relativeBezelWidth,2,MPI_FLOAT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&arrangement,1,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&stereo,1,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
    }

    /*! open an MPI port and wait for the client(s) to connect to this
        port after this function terminates, all outward facing procs
        (either the head node, or all display nodes if no head node is
        being used) should have an proper MPI communicator set up to
        talk to the client rank(s) */
    MPI::Group Server::waitForConnection(const MPI::Group &outwardFacingGroup,
                                         int desiredInfoPortNum)
    {
      MPI_Comm outside;
      outwardFacingGroup.barrier();
//       printf("outward facing rank %i/%i waiting for outside connection\n",
//              outwardFacingGroup.rank,outwardFacingGroup.size);
      outwardFacingGroup.barrier();

      /* open a port, and publish its name */
      char portName[MPI_MAX_PORT_NAME];
      if (outwardFacingGroup.rank == 0) {
        MPI_CALL(Open_port(MPI_INFO_NULL,portName));
        printf("diplay wald waiting for connection on MPI port %s\n",
               portName);
      }

      if (outwardFacingGroup.rank == 0) 
        /* open the port that we give display wall info on
           (capabilities and MPI port tname of the service); do that
           only on rank 0 (or head node, if used) */
        openInfoPort(portName,wallConfig,desiredInfoPortNum);
        

      /* accept / wait for outside connection on this port */
      MPI_CALL(Comm_accept(portName,MPI_INFO_NULL,0,outwardFacingGroup.comm,&outside));
      if (outwardFacingGroup.rank == 0) {
        printf("communication established...\n");
      }
      sendConfigToClient(MPI::Group(outside),outwardFacingGroup,wallConfig);

      outwardFacingGroup.barrier();

      /* and return the inter-communicator to the outside */
      return MPI::Group(outside);
    };
    
    /*! allocate the frame buffers for left/right eye and recv/display, respectively */
    void Server::allocateFrameBuffers()
    {
      assert(recv_l == NULL);
      assert(disp_l == NULL);
      assert(recv_r == NULL);
      assert(disp_r == NULL);

      // // me.barrier();
      // if (outsdide.rank == 0)
      //   printf("#osp:dw: allocating frame buffers of %ix%i pixels\n",
      //          wallConfig.pixelsPerDisplay.x,wallConfig.pixelsPerDisplay.y);
      // // me.barrier();

      const int pixelsPerBuffer = wallConfig.pixelsPerDisplay.product();
      recv_l = new uint32_t[pixelsPerBuffer];
      disp_l = new uint32_t[pixelsPerBuffer];
      if (wallConfig.stereo) {
        recv_r = new uint32_t[pixelsPerBuffer];
        disp_r = new uint32_t[pixelsPerBuffer];
      }
      // me.barrier();
      // if (me.rank == 0)
      //   printf("#osp:dw: frame buffers allocated (across all ranks)\n");
      // me.barrier();
    }

    /*! in dispather.cpp - the dispatcher that receives tiles on the
        head node, and then dispatches them to the actual tile
        receivers */
    void runDispatcher(const MPI::Group &outside,
                       const MPI::Group &displays,
                       const WallConfig &wallConfig);

          // setupCommunications(this->wallConfig,
          //                     this->hasHeadNode,
          //                     this->me,
          //                     this->desiredInfoPortNum);
    /*! note: this runs in its own thread */
    void Server::setupCommunications()
// const WallConfig &wallConfig,
//                                      bool hasHeadNode,
//                                      const MPI::Group &world,
//                                      int desiredInfoPortNum)
    {
      const MPI::Group &world = me;
      // =======================================================
      /* create inter- and intra-comms to communicate between displays
         and, if applicable, the dispathcer running ont he head
         node. if no head node is used the dispatcher comm is a
         COMM_NULL */

      // intercomm to the dispatcher, if this is a display node;
      // intracomm containing onyl the dispatche3r node if one exists,
      // or invalid if we're not running w/ a head node.
      MPI::Group dispatchGroup;

      if (hasHeadNode) {
        MPI_Comm intraComm, interComm;
        MPI_CALL(Comm_split(world.comm,1+(world.rank>0),world.rank,&intraComm));
        
        if (world.rank == 0) {
          dispatchGroup = MPI::Group(intraComm);
          MPI_Intercomm_create(intraComm,0,world.comm, 1, 1, &interComm); 
          displayGroup = MPI::Group(interComm);
        } else {
          displayGroup = MPI::Group(intraComm);
          MPI_Intercomm_create(intraComm,0,world.comm, 0, 1, &interComm); 
          dispatchGroup = MPI::Group(interComm);
        }
      } else {
        displayGroup = world.dup();
      }
//       printf("world rank %i/%i: dispatcher rank %i/%i, display rank %i/%i\n",
//              world.rank,world.size,
//              dispatchGroup.rank,dispatchGroup.size,
//              displayGroup.rank,displayGroup.size);
      MPI_CALL(Barrier(world.comm));

      commThreadIsReady.unlock();
      if (hasHeadNode) {
        if (world.rank == 0) {
          // =======================================================
          // DISPATCHER
          // =======================================================
          MPI::Group outsideConnection
            = waitForConnection(dispatchGroup,desiredInfoPortNum);
          runDispatcher(outsideConnection,displayGroup,wallConfig);
        } else {
          // =======================================================
          // TILE RECEIVER
          // =======================================================
          canStartProcessing.lock();
          MPI::Group incomingTiles = dispatchGroup;
          processIncomingTiles(incomingTiles);
        }
      } else {
        // =======================================================
        // TILE RECEIVER
        // =======================================================
        canStartProcessing.lock();
        MPI::Group incomingTiles
          = waitForConnection(displayGroup,desiredInfoPortNum);
        processIncomingTiles(incomingTiles);
      }
    }
    
    void startDisplayWallService(const MPI_Comm comm,
                                 const WallConfig &wallConfig,
                                 bool hasHeadNode,
                                 DisplayCallback displayCallback,
                                 void *objectForCallback,
                                 int desiredInfoPortNum)
    {
      assert(Server::singleton == NULL);
      Server::singleton = new Server(MPI::Group(comm),wallConfig,hasHeadNode,
                                     displayCallback,objectForCallback,
                                     desiredInfoPortNum);
    }

    Server::Server(const MPI::Group &world,
                   const WallConfig &wallConfig,
                   const bool hasHeadNode,
                   DisplayCallback displayCallback,
                   void *objectForCallback,
                   int desiredInfoPortNum)
      : me(world.dup()),
        wallConfig(wallConfig),
        hasHeadNode(hasHeadNode),
        displayCallback(displayCallback),
        objectForCallback(objectForCallback),
        // commThread(NULL),
        numWrittenThisFrame(0),
        numExpectedThisFrame(wallConfig.displayPixelCount()),
        recv_l(NULL),
        recv_r(NULL),
        disp_l(NULL),
        disp_r(NULL),
        desiredInfoPortNum(desiredInfoPortNum)
    {
      commThreadIsReady.lock();
      canStartProcessing.lock();
#if 1
      commThread = std::thread([this]() {
           this->setupCommunications();
        });
      commThreadIsReady.lock();
#endif
 
      if (hasHeadNode && me.rank == 0) {
        /* if this is the head node we wait here until everything is
           done; this prevents the comm thread from dying when we
           return to main - unlike other ranks the head node will NOT
           open a window and enter a windowing loop.... */
        commThread.join();
      }

      canStartProcessing.unlock();

      /* MIGHT want to wait for threads to be started here  */

      /* need to add some shutdown code that re-joins that thread when
         all is done */
    }
    
  } // ::ospray::dw
} // ::ospray
