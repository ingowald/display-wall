#include "CompressedTile.h"

namespace ospray {
  namespace dw {

    struct CompressedTileHeader {
      box2i region;
      int32_t frameID;
      unsigned char payload[0];
    };

    CompressedTile::CompressedTile() 
      : fromRank(-1), 
        numBytes(-1), 
        data(NULL) 
    {}

    CompressedTile::~CompressedTile() 
    { 
      if (data) delete[] data; 
    }

    void CompressedTile::encode(const PlainTile &tile)
    {
      const vec2i begin = tile.region.lower;
      const vec2i end = tile.region.upper;

      int numInts = 5+(end-begin).product();
      assert(this->data = NULL);
      this->numBytes = numInts*sizeof(int);
      this->data = new unsigned char[this->numBytes];
      int *write = (int *)this->data;
      *write++ = begin.x;
      *write++ = begin.y;
      *write++ = end.x;
      *write++ = end.y;
      *write++ = tile.frameID;
      const int *line = (const int *)tile.pixel;
      for (int iy=begin.y;iy<end.y;iy++) {
        const int *in = line;
        for (int ix=begin.x;ix<end.x;ix++)
          *write++ = *line++;
        line += tile.pitch;
      }
    }

    /*! send the tile to the given rank in the given group */
    void CompressedTile::sendTo(const MPI::Group &group, const int rank) const
    {
      MPI_CALL(Send(data,numBytes,MPI_BYTE,rank,0,group.comm));
    }

    /*! receive one tile from the outside communicator */
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
      data = new unsigned char[numBytes];
      MPI_CALL(Recv(data,numBytes,MPI_BYTE,status.MPI_SOURCE,status.MPI_TAG,
                    outside.comm,&status));
    }

  } // ::ospray::dw
} // ::ospray
