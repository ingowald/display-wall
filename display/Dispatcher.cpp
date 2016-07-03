#include "MPI.h"

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    /*! in dispather.cpp - the dispatcher that receives tiles on the
        head node, and then dispatches them to the actual tile
        receivers */
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

    
  } // ::ospray::dw
} // ::ospray
