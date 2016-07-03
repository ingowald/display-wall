#pragma once 

#include "common/box.h"
#include "common/MPI.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

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
    

  } // ::ospray::dw
} // ::ospray
