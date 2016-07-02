
#include "MPI.h"
#include "GlutWindow.h"
//#include "ospray/common/Thread.h"
#include <thread>

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;

    // default settings
    bool hasHeadNode = false;
    vec2i windowSize(320,240);

    void runDispatcher()
    {
      while(1);
    }


    // struct AcceptThread : public Thread 
    // {
    //   AcceptThread
    //   virtual void run()
    //   {
    //   }

    //   MPI::Group receivers;
    // }


    MPI::Group establishOutsideConnection(MPI::Group &worldComm,
                                          bool hasHeadNode,
                                          MPI::Group &receiverComm,
                                          MPI::Group &displayComm)
    {
      receivers.barrier();
      if (receivers.rank == 0) {
        // new std::thread(acceptThread,&re);
        char portName[MPI_MAX_PORT_NAME+1];
        MPI_CALL(Open_port(MPI_INFO_NULL,portName));
        PRINT(portName);
      }
     
      // =======================================================
      /* launch the dispatcher, if we need one ...*/
      // =======================================================
      if (hasHeadNode && world.rank == 0)
        launchDispatcher(toDispatcher,toDisplay);
      MPI_CALL(Barrier(world.comm));
 
      return MPI::Group();
    }
    
    void launchDispatcher(MPI::Group &outside,
                          MPI::Group &displays)
    {
      std::thread *dispatcherThread = new std::thread([=]() {
          std::cout << "Running the dispatcher thread ..." << std::endl;
          PRINT(outside.size);
          PRINT(displays.size);
          while (1) {
            MPI_Status status;
            MPI_CALL(Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,outside.comm,&status));
            cout << "incoming from " << status.MPI_SOURCE << flush;
            int numBytes;
            MPI_CALL(Get_count(&status,MPI_BYTE,&numBytes));
            cout << "... has " << numBytes << " bytes" << endl;

            char buf[numBytes];
            MPI_CALL(Recv(buf,numBytes,MPI_BYTE,status.MPI_SOURCE,status.MPI_TAG,
                          outside.comm,&status));
          };
        });
    }
    
    extern "C" int main(int ac, char **av)
    {
      glutInit(&ac, (char **) av);
      MPI::init(ac,av);
      MPI::Group world(MPI_COMM_WORLD);
      std::string connectToPort = "";

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

      // =======================================================
      /* create inter- and intra-comms to communicate between displays
         and, if applicable, the dispathcer running ont he head
         node. if no head node is used the dispatcher comm is a
         COMM_NULL */

      // dispathcer to communicate with other dispalys - either a
      // intracomm if this is a display node, or a intracomm if this
      // is the head node
      MPI::Group toDisplay;
      // intercomm to the dispatcher, if this is a display node;
      // intracomm containing onyl the dispatche3r node if one exists,
      // or invalid if we're not running w/ a head node.
      MPI::Group toDispatcher;
      if (!hasHeadNode) {
        toDisplay = world.dup();
      } else {
        MPI_Comm intraComm, interComm;
        MPI_CALL(Comm_split(world.comm,1+(world.rank>0),world.rank,&intraComm));

        if (world.rank == 0) {
          toDispatcher = MPI::Group(intraComm);
          MPI_Intercomm_create(intraComm,0,world.comm, 1, 1, &interComm); 
          toDisplay = MPI::Group(interComm);
        } else {
          toDisplay = MPI::Group(intraComm);
          MPI_Intercomm_create(intraComm,0,world.comm, 0, 1, &interComm); 
          // PRINT(interComm);
          toDispatcher = MPI::Group(interComm);
        }

      }
      printf("world rank %i/%i: dispatcher rank %i/%i, display rank %i/%i\n",
             world.rank,world.size,
             toDispatcher.rank,toDispatcher.size,
             toDisplay.rank,toDisplay.size);
      MPI_CALL(Barrier(world.comm));
      
      // =======================================================
      /* launch the threads that do the outside connection ...*/
      // =======================================================
      if (world.rank == 0)
        std::cout << "internal communicators established ... "
                  << "now establishing external connection." << endl; 
      MPI_CALL(Barrier(world.comm));

      establishOutsideConnection(world,hasHeadNode,toDispatcher,toDisplay);
      MPI_CALL(Barrier(world.comm));

      while (1);
      if (hasHeadNode && world.rank == 0) {
        /* do not open any windows ...*/
        printf("creating a head node ... not opening any window on that\n");

        runDispatcher();
      } else {

        const char *title = "display wall window";
        GlutWindow glutWindow(windowSize,title);
        
        FrameBuffer *receiveFB = new FrameBuffer(glutWindow.getSize(),glutWindow.doesStereo());
        FrameBuffer *displayFB = new FrameBuffer(glutWindow.getSize(),glutWindow.doesStereo());
        
        glutWindow.run();
      }

      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
