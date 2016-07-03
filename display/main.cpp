
#include "MPI.h"
#include "GlutWindow.h"
//#include "ospray/common/Thread.h"
#include <thread>

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    // default settings
    bool hasHeadNode = false;
    vec2i windowSize(320,240);

    MPI::Group waitForConnection(MPI::Group &inside)
    {
      PING;
      return MPI::Group();
    };
    
    void runDispatcher(MPI::Group &outside,
                       MPI::Group &displays)
    {
      // std::thread *dispatcherThread = new std::thread([=]() {
      std::cout << "Running the dispatcher thread ..." << std::endl;
      PRINT(outside.size);
      PRINT(displays.size);
      while (1) {
        throw std::runtime_error("not implemented");
        // MPI_Status status;
        // MPI_CALL(Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,outside.comm,&status));
        // cout << "incoming from " << status.MPI_SOURCE << flush;
        // int numBytes;
        // MPI_CALL(Get_count(&status,MPI_BYTE,&numBytes));
        // cout << "... has " << numBytes << " bytes" << endl;

        // char buf[numBytes];
        // MPI_CALL(Recv(buf,numBytes,MPI_BYTE,status.MPI_SOURCE,status.MPI_TAG,
        //               outside.comm,&status));
      };
      // });
    }

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
      void receiveOne(MPI::Group &inside, MPI::Group &outside);
    };
    
    void CompressedTile::receiveOne(MPI::Group &outside, MPI::Group &me)
    {
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
      std::string connectToPort = "";
      MPI::Group world(MPI_COMM_WORLD);

      for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        if (arg == "--head-node" || arg == "-hn") {
          hasHeadNode = true;
        } else if (arg == "--no-head-node" || arg == "-nhn") {
          hasHeadNode = false;
        } else if (arg[0] == '-') {
          throw std::runtime_error("unkonwn arg "+arg);
        } else connectToPort = arg;
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
