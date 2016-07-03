
#include "MPI.h"
#include "GlutWindow.h"
//#include "ospray/common/Thread.h"
#include <thread>

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    std::string portNameFile = "~/.ospDisplayWald.port";

    // default settings
    bool hasHeadNode = false;
    vec2i windowSize(320,240);

    void sendConfigToClient(const MPI::Group &outside, const MPI::Group &me)
    {
      printf("sending config to client ... not yet implemented...\n");
    }

    MPI::Group waitForConnection(MPI::Group &me)
    {
      MPI_Comm outside;
      me.barrier();
      printf("outward facing rank %i/%i waiting for outside connection\n",
             me.rank,me.size);
      me.barrier();

      char portName[MPI_MAX_PORT_NAME];
      if (me.rank == 0) {
        MPI_CALL(Open_port(MPI_INFO_NULL,portName));
        printf("diplay wald waiting for connection on MPI port %s\n",
               portName);
      }
      MPI_CALL(Comm_accept(portName,MPI_INFO_NULL,0,me.comm,&outside));
      if (me.rank == 0) {
        printf("communication established...\n");
        sendConfigToClient(MPI::Group(outside),me);
        FILE *file = fopen(portNameFile.c_str(),"w");
        if (!file) 
          throw std::runtime_error("could not open "+portNameFile+
                                   " for writing port name");
        fprintf(file,"%s",portName);
        fclose(file);
        printf("port name writtten to %s\n",portNameFile.c_str());
      }
      me.barrier();
      return MPI::Group(outside);
    };

    /*! in dispather.cpp - the dispatcher that receives tiles on the
        head node, and then dispatches them to the actual tile
        receivers */
    void runDispatcher(MPI::Group &outside,
                       MPI::Group &displays);

    struct CompressedTile {
      CompressedTile() : fromRank(-1), numBytes(-1), data(NULL) {}
      ~CompressedTile() { if (data) delete[] data; }
      struct Header {
        box2i region;
        int32_t frameID;
        char payload[0];
      };
      char *data;
      int fromRank;
      int numBytes;

      /*! receive one tile from the outside communicator */
      void receiveOne(MPI::Group &outside, MPI::Group &me);
    };
    
    void CompressedTile::receiveOne(MPI::Group &outside, MPI::Group &me)
    {
      PING;
      printf("receiveone me %i/%i outside %i/%i\n",me.rank,me.size,outside.rank,outside.size);
      MPI_Status status;
      MPI_CALL(Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,outside.comm,&status));
      fromRank = status.MPI_SOURCE;
      MPI_CALL(Get_count(&status,MPI_BYTE,&numBytes));        
      printf("%i/%i incoming from %i, %i bytes\n",me.rank,me.size,
             status.MPI_SOURCE,numBytes);
      data = new char[numBytes];
      MPI_CALL(Recv(data,numBytes,MPI_BYTE,status.MPI_SOURCE,status.MPI_TAG,
                    outside.comm,&status));
    }
    
    void processIncomingTiles(MPI::Group &outside,MPI::Group &me)
    {
      cout << "running tile receiver ..." << endl;
      while (1) {
        // -------------------------------------------------------
        // receive one tiles
        // -------------------------------------------------------
        CompressedTile tile;
        tile.receiveOne(outside,me);
        throw std::runtime_error("need to decode tile here ... ");
      }
    }

    /*! note: this runs in its own thread */
    void setupCommunications(GlutWindow *window,
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
          runDispatcher(outsideConnection,displayGroup);
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
        } else {
          throw std::runtime_error("unkonwn arg "+arg);
        } 
      }

      const char *title = "display wall window";
      GlutWindow glutWindow(windowSize,title);

      if (hasHeadNode && world.rank == 0) {
        cout << "running a dedicated headnode on rank 0; not creating a window there" << endl;
      } else {
        glutWindow.create();
      }
      
      std::thread commThread([&]() {
          setupCommunications(&glutWindow,hasHeadNode,world);
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
