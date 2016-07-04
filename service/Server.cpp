#include "Server.h"
#include "../common/CompressedTile.h"

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    std::string portNameFile = ".ospDisplayWald.port";

    // default settings
    // bool hasHeadNode = false;
    // vec2i windowSize(320,240);
    // vec2i numDisplays(0,0);

    void sendConfigToClient(const MPI::Group &outside, 
                            const MPI::Group &me,
                            const WallConfig &wallConfig)
    {
      vec2i numDisplays = wallConfig.numDisplays;
      vec2i pixelsPerDisplay = wallConfig.pixelsPerDisplay;
      int arrangement = wallConfig.displayArrangement;
      int stereo      = wallConfig.stereo;
      /*! if we're the head node, let's 'fake' a single display to the client */
      if (me.size == 1) {
        pixelsPerDisplay = pixelsPerDisplay * numDisplays;
        numDisplays = vec2i(1);
      }

      MPI_CALL(Bcast(&numDisplays,2,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,2,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&arrangement,1,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      MPI_CALL(Bcast(&stereo,1,MPI_INT,
                     me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
    }

    MPI::Group waitForConnection(MPI::Group &me, const WallConfig &wallConfig)
    {
      MPI_Comm outside;
      me.barrier();
      printf("outward facing rank %i/%i waiting for outside connection\n",
             me.rank,me.size);
      me.barrier();

      /* open a port, and publish its name */
      char portName[MPI_MAX_PORT_NAME];
      if (me.rank == 0) {
        MPI_CALL(Open_port(MPI_INFO_NULL,portName));
        printf("diplay wald waiting for connection on MPI port %s\n",
               portName);
        FILE *file = fopen(portNameFile.c_str(),"w");
        if (!file) 
          throw std::runtime_error("could not open "+portNameFile+
                                   " for writing port name");
        fprintf(file,"%s",portName);
        fclose(file);
        printf("port name writtten to %s\n",portNameFile.c_str());
      }
      
      /* accept / wait for outside connection on this port */
      MPI_CALL(Comm_accept(portName,MPI_INFO_NULL,0,me.comm,&outside));
      if (me.rank == 0) {
        printf("communication established...\n");
      }
      sendConfigToClient(MPI::Group(outside),me,wallConfig);
      me.barrier();

      /* and return the inter-communicator to the outside */
      return MPI::Group(outside);
    };

    /*! in dispather.cpp - the dispatcher that receives tiles on the
        head node, and then dispatches them to the actual tile
        receivers */
    void runDispatcher(const MPI::Group &outside,
                       const MPI::Group &displays,
                       const WallConfig &wallConfig);

    void processIncomingTiles(MPI::Group &outside,MPI::Group &me);

    /*! note: this runs in its own thread */
    void setupCommunications(const WallConfig &wallConfig,
                             bool hasHeadNode,
                             const MPI::Group &world)
    {
      PING;
      // =======================================================
      /* create inter- and intra-comms to communicate between displays
         and, if applicable, the dispathcer running ont he head
         node. if no head node is used the dispatcher comm is a
         COMM_NULL */

      // dispathcer to communicate with other dispalys - either a
      // intracomm if this is a display node, or a intracomm if this
      // is the head node
      MPI::Group displayGroup;
      PING;
      // intercomm to the dispatcher, if this is a display node;
      // intracomm containing onyl the dispatche3r node if one exists,
      // or invalid if we're not running w/ a head node.
      PING;
      MPI::Group dispatchGroup;
      PING;

      if (hasHeadNode) {
      PING;
        MPI_Comm intraComm, interComm;
        MPI_CALL(Comm_split(world.comm,1+(world.rank>0),world.rank,&intraComm));
        
        if (world.rank == 0) {
          dispatchGroup = MPI::Group(intraComm);
          MPI_Intercomm_create(intraComm,0,world.comm, 1, 1, &interComm); 
          displayGroup = MPI::Group(interComm);
        } else {
      PING;
          displayGroup = MPI::Group(intraComm);
          MPI_Intercomm_create(intraComm,0,world.comm, 0, 1, &interComm); 
          // PRINT(interComm);
          dispatchGroup = MPI::Group(interComm);
        }
      } else {
        displayGroup = world.dup();
      }
      PING;
      printf("world rank %i/%i: dispatcher rank %i/%i, display rank %i/%i\n",
             world.rank,world.size,
             dispatchGroup.rank,dispatchGroup.size,
             displayGroup.rank,displayGroup.size);
      MPI_CALL(Barrier(world.comm));



      if (hasHeadNode) {
        if (world.rank == 0) {
          // =======================================================
          // DISPATCHER
          // =======================================================
          MPI::Group outsideConnection = waitForConnection(dispatchGroup,wallConfig);
          runDispatcher(outsideConnection,displayGroup,wallConfig);
        } else {
          // =======================================================
          // TILE RECEIVER
          // =======================================================
          MPI::Group incomingTiles = dispatchGroup;
          processIncomingTiles(incomingTiles,displayGroup);
        }
      } else {
        // =======================================================
        // TILE RECEIVER
        // =======================================================
        MPI::Group incomingTiles = waitForConnection(displayGroup,wallConfig);
        processIncomingTiles(incomingTiles,displayGroup);
      }
    }
    
    DisplayCallback displayCallback = NULL;
    void *objectForCallback = NULL;

    void startDisplayWallService(const MPI_Comm comm,
                                 const WallConfig &wallConfig,
                                 bool hasHeadNode,
                                 DisplayCallback displayCallback,
                                 void *objectForCallback)
    {
      MPI::Group world(comm);
      MPI::Group me = world.dup();
      ospray::dw::displayCallback = displayCallback;
      ospray::dw::objectForCallback = objectForCallback;
      std::thread *commThread = new std::thread([=]() {
          PING;
          setupCommunications(wallConfig,hasHeadNode,me);
        });

      if (hasHeadNode && me.rank == 0) {
        /* if this is the head node we wait here until everything is
           done; this prevents the comm thread from dying when we
           return to main - unlike other ranks the head node will NOT
           open a window and enter a windowing loop.... */
        commThread->join();
      }
      /* MIGHT want to wait for threads to be started here  */

      /* need to add some shutdown code that re-joins that thread when
         all is done */
    }
    
  } // ::ospray::dw
} // ::ospray
