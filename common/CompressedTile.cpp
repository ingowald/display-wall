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

#include "CompressedTile.h"
#include <atomic>

#if TURBO_JPEG
# include "turbojpeg.h"
# define JPEG_QUALITY 100
#endif

namespace ospray {
  namespace dw {

#if TURBO_JPEG
    void *CompressedTile::createCompressor() { return (void *)tjInitCompress(); }
    void *CompressedTile::createDecompressor() { return (void *)tjInitDecompress(); }
    void CompressedTile::freeCompressor(void *compressor) { tjDestroy((tjhandle)compressor); }
    void CompressedTile::freeDecompressor(void *decompressor) { tjDestroy((tjhandle)decompressor); }
#else
    void *CompressedTile::createCompressor() { return NULL; }
    void *CompressedTile::createDecompressor() { return NULL; }
    void CompressedTile::freeCompressor(void *) {}
    void CompressedTile::freeDecompressor(void *) {}
#endif

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

    void CompressedTile::encode(void *compressor, const PlainTile &tile)
    {
      assert(tile.pixel);
      const vec2i begin = tile.region.lower;
      const vec2i end = tile.region.upper;

      int numPixels = (end-begin).product();
      this->numBytes = sizeof(CompressedTileHeader)+numPixels*sizeof(int);
      this->data = new unsigned char[this->numBytes];
      assert(this->data != NULL);
      CompressedTileHeader *header = (CompressedTileHeader *)this->data;
      header->region.lower = begin;
      header->region.upper = end;

#if TURBO_JPEG                       
      unsigned char *jpegBuffer = header->payload; //NULL;
      size_t jpegSize = numPixels*sizeof(int);
      int rc = tjCompress2((tjhandle)compressor, (unsigned char *)tile.pixel,
                           tile.size().x,tile.pitch*sizeof(int),tile.size().y,
                           TJPF_BGRX, &jpegBuffer,&jpegSize,TJSAMP_444,JPEG_QUALITY,0);
      this->numBytes = jpegSize + sizeof(*header);
      // printf("compress %i: %li->%li bytes\n",rc,numPixels*sizeof(int),jpegSize);
#else
      uint32_t *out = (uint32_t *)header->payload;
      const uint32_t *in = (const uint32_t *)tile.pixel;
      for (uint32_t iy=begin.y;iy<end.y;iy++) {
        for (uint32_t ix=begin.x;ix<end.x;ix++) {
          *out++ = *in++;
        }

        in += (tile.pitch-(end.x-begin.x));
      }
#endif
    }
    
    void CompressedTile::decode(void *decompressor, PlainTile &tile)
    {
      const CompressedTileHeader *header = (const CompressedTileHeader *)data;
      tile.region = header->region;
      vec2i size = tile.region.size();
      assert(tile.pixel != NULL);
#if TURBO_JPEG                       
      size_t jpegSize = this->numBytes-sizeof(*header);
      int rc = tjDecompress2((tjhandle)decompressor, (unsigned char *)header->payload,
                             this->numBytes-sizeof(*header),
                             (unsigned char*)tile.pixel,
                             size.x,tile.pitch*sizeof(int),size.y,
                             TJPF_BGRX, 0);
#else
      uint32_t *out = tile.pixel;
      uint32_t *in = (uint32_t *)(data+sizeof(CompressedTileHeader));
      for (int iy=0;iy<size.y;iy++)
        for (int ix=0;ix<size.x;ix++) {
          *out++ = *in++;
        }
#endif
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
      static std::atomic<int> tileID;
      int myTileID = tileID++;
      MPI_CALL(Send(data,numBytes,MPI_BYTE,rank,myTileID,group.comm));
    }

    /*! receive one tile from the outside communicator */
    void CompressedTile::receiveOne(const MPI::Group &outside)
    {
      // printf("receiveone from outside %i/%i\n",outside.rank,outside.size);
      MPI_Status status;
      MPI_CALL(Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,outside.comm,&status));
      fromRank = status.MPI_SOURCE;
      MPI_CALL(Get_count(&status,MPI_BYTE,&numBytes));        
      // printf("incoming from %i, %i bytes\n",status.MPI_SOURCE,numBytes);
      data = new unsigned char[numBytes];
      MPI_CALL(Recv(data,numBytes,MPI_BYTE,status.MPI_SOURCE,status.MPI_TAG,
                    outside.comm,&status));
    }

  } // ::ospray::dw
} // ::ospray
