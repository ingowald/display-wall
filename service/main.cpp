
#include "../common/MPI.h"
#include "../common/CompressedTile.h"
#include "../common/WallConfig.h"
#include "GlutWindow.h"
//#include "ospray/common/Thread.h"
#include <thread>

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    std::string portNameFile = ".ospDisplayWald.port";

    // default settings
    bool hasHeadNode = false;
    vec2i windowSize(320,240);
    vec2i numDisplays(0,0);

    void sendConfigToClient(const MPI::Group &outside, const MPI::Group &me)
    {
      if (me.size == 1) {
        // we're the head node running a dispatcher - send a fake 'single display'
        vec2i fake_windowSize = numDisplays*windowSize;
        vec2i fake_numDisplays = vec2i(1);
        MPI_CALL(Bcast(&fake_numDisplays,sizeof(fake_numDisplays),MPI_BYTE,
                       me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
        MPI_CALL(Bcast(&fake_windowSize,sizeof(fake_windowSize),MPI_BYTE,
                       me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      } else {
        MPI_CALL(Bcast(&numDisplays,sizeof(numDisplays),MPI_BYTE,
                       me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
        MPI_CALL(Bcast(&windowSize,sizeof(windowSize),MPI_BYTE,
                       me.rank==0?MPI_ROOT:MPI_PROC_NULL,outside.comm));
      }
    }

    MPI::Group waitForConnection(MPI::Group &me)
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
      sendConfigToClient(MPI::Group(outside),me);
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
    void setupCommunications(GlutWindow *window,
                             const WallConfig &wallConfig,
                             bool hasHeadNode,
                             MPI::Group &world)
    {
      // =======================================================
      /* create inter- and intra-comms to communicate between displays
         and, if applicable, the dispathcer running ont he head
         node. if no head node is used the dispatcher comm is a
         COMM_NULL */

      // dispathcer to communicate with other dispalys - either a
      // intracomm if this is a display node, or a intracomm if this
      // is the head node
      MPI::Group displayGroup;
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
          // PRINT(interComm);
          dispatchGroup = MPI::Group(interComm);
        }
      } else {
        displayGroup = world.dup();
      }

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
          MPI::Group outsideConnection = waitForConnection(dispatchGroup);
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
        MPI::Group incomingTiles = waitForConnection(displayGroup);
        processIncomingTiles(incomingTiles,displayGroup);
      }
    }
    
    void usage(const std::string &err)
    {
      if (!err.empty()) {
        cout << "Error: " << err << endl << endl;
      }
      cout << "usage: ./ospDisplayWald [args]*" << endl << endl;
      cout << "w/ args: " << endl;
      cout << "--width|-w <numDisplays.x>     - num displays in x direction" << endl;
      cout << "--height|-h <numDisplays.y>    - num displays in y direction" << endl;
      cout << "--[no-]head-node | -[n]hn      - use / do not use dedicated head node" << endl;
      exit(!err.empty());
    }

    extern "C" int main(int ac, char **av)
    {
      glutInit(&ac, (char **) av);
      MPI::init(ac,av);
      MPI::Group world(MPI_COMM_WORLD);

      for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        if (arg == "--head-node" || arg == "-hn") {
          hasHeadNode = true;
        } else if (arg == "--no-head-node" || arg == "-nhn") {
          hasHeadNode = false;
        } else if (arg == "--width" || arg == "-w") {
          numDisplays.x = atoi(av[++i]);
        } else if (arg == "--height" || arg == "-h") {
          numDisplays.y = atoi(av[++i]);
        } else {
          usage("unkonwn arg "+arg);
        } 
      }

      if (numDisplays.x < 1) 
        usage("no display wall width specified (--width <w>)");
      if (numDisplays.y < 1) 
        usage("no display wall height specified (--heigh <h>)");
      if (world.size != numDisplays.x*numDisplays.y+hasHeadNode)
        throw std::runtime_error("invalid number of ranks for given display/head node config");
      WallConfig wallConfig(numDisplays,windowSize);

      const char *title = "display wall window";
      GlutWindow glutWindow(windowSize,title);

      if (hasHeadNode && world.rank == 0) {
        cout << "running a dedicated headnode on rank 0; not creating a window there" << endl;
      } else {
        glutWindow.create();
      }
      
      std::thread commThread([&]() {
          setupCommunications(&glutWindow,wallConfig,hasHeadNode,world);
        });

      if (hasHeadNode && world.rank == 0) {
        /* no window on head node */
      } else {
        glutWindow.run();
      }
      commThread.join();
      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
