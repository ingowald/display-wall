#include "CompressedTile.h"

namespace ospray {
  namespace dw {

    struct CompressedTileHeader {
      box2i region;
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

      int numInts = 4+(end-begin).product();
      assert(this->data = NULL);
      this->numBytes = numInts*sizeof(int);
      this->data = new unsigned char[this->numBytes];
      int *write = (int *)(this->data+sizeof(CompressedTileHeader));
      *write++ = begin.x;
      *write++ = begin.y;
      *write++ = end.x;
      *write++ = end.y;
      const int *line = (const int *)tile.pixel;
      for (int iy=begin.y;iy<end.y;iy++) {
        const int *in = line;
        for (int ix=begin.x;ix<end.x;ix++)
          *write++ = *line++;
        line += tile.pitch;
      }
    }

    void CompressedTile::decode(PlainTile &tile)
    {
      const CompressedTileHeader *header = (const CompressedTileHeader *)data;
      tile.region = header->region;
      vec2i size = tile.region.upper-tile.region.lower;
      tile.pitch = size.x;
      int numInts = size.x*size.y;
      assert(tile.pixel == NULL);
      tile.pixel = new uint32_t[numInts];
      uint32_t *out = tile.pixel;
      uint32_t *in = (uint32_t *)(data+sizeof(CompressedTileHeader));
      for (int iy=0;iy<size.y;iy++)
        for (int ix=0;ix<size.x;ix++) {
          *out++ = *in++;
        }
    }

    /*! get region that this tile corresponds to */
    box2i CompressedTile::getRegion() const
    {
      const CompressedTileHeader *header = (const CompressedTileHeader *)data;
      assert(header);
      return header->region;
    }
    
    /*! send the tile to the given rank in the given group */
    void CompressedTile::sendTo(const MPI::Group &group, const int rank) const
    {
      PING;
      MPI_CALL(Send(data,numBytes,MPI_BYTE,rank,0,group.comm));
    }

    /*! receive one tile from the outside communicator */
    void CompressedTile::receiveOne(const MPI::Group &outside)
    {
      printf("receiveone from outside %i/%i\n",outside.rank,outside.size);
      MPI_Status status;
      MPI_CALL(Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,outside.comm,&status));
      fromRank = status.MPI_SOURCE;
      MPI_CALL(Get_count(&status,MPI_BYTE,&numBytes));        
      printf("incoming from %i, %i bytes\n",status.MPI_SOURCE,numBytes);
      data = new unsigned char[numBytes];
      MPI_CALL(Recv(data,numBytes,MPI_BYTE,status.MPI_SOURCE,status.MPI_TAG,
                    outside.comm,&status));
    }

  } // ::ospray::dw
} // ::ospray
