/* 
Copyright (c) 2016 Ingo Wald

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies obf the Software, and to permit persons to whom the Software is
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

#include "Client.h"
#include "../common/Socket.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;
        
    using std::cout; 
    using std::endl;
    using std::flush;

    /*! read a service info from a given hostName:port. The service
        has to already be running on that port */
    void ServiceInfo::getFrom(const std::string &hostName,
                              const int portNo)
    {
      socket_t sock = connect(hostName.c_str(),portNo);
      if (!sock)
        throw std::runtime_error("could not create display wall connection!");

      mpiPortName = read_string(sock);
      totalPixelsInWall.x = read_int(sock);
      totalPixelsInWall.y = read_int(sock);
      stereo = read_int(sock);
      close(sock);
    }

    
    Client::Client(const MPI::Group &me,
                   const std::string &portName)
      : me(me), wallConfig(NULL)
    {
      establishConnection(portName);
      receiveDisplayConfig();

      assert(wallConfig);
      if (me.rank == 0)
        wallConfig->print();
    }
    
    void Client::receiveDisplayConfig() 
    {
      vec2i numDisplays;
      vec2i pixelsPerDisplay;
      int arrangement;
      int stereo;
      vec2f relativeBezelWidth;
      if (me.rank == 0) 
        cout << "waiting for service to tell us the display wall config..." << endl;
      MPI_CALL(Bcast(&numDisplays,2,MPI_INT,0,displayGroup.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,2,MPI_INT,0,displayGroup.comm));
      MPI_CALL(Bcast(&relativeBezelWidth,2,MPI_FLOAT,0,displayGroup.comm));
      MPI_CALL(Bcast(&arrangement,1,MPI_INT,0,displayGroup.comm));
      MPI_CALL(Bcast(&stereo,1,MPI_INT,0,displayGroup.comm));
      wallConfig = new WallConfig(numDisplays,pixelsPerDisplay,
                                  relativeBezelWidth,
                                  (WallConfig::DisplayArrangement)arrangement,
                                  stereo);
    }

    /*! establish connection between 'me' and the remote service */
    void Client::establishConnection(const std::string &portName)
    {
      me.barrier();
      if (me.rank == 0) {
        if (portName.empty())
          throw std::runtime_error("no mpi port name provided to establish connection");
        cout << "#osp.dw: trying to connect to MPI port '"<<portName << "'" << endl;
      }
      MPI_Comm newComm;
      MPI_CALL(Comm_connect(portName.c_str(),MPI_INFO_NULL,0,me.comm,&newComm));
      this->displayGroup = MPI::Group(newComm);
      
      if (me.rank == 0)
        cout << "connection established..." << endl;
      me.barrier();
    }

    vec2i Client::totalPixelsInWall() const 
    {
      assert(wallConfig);
      return wallConfig->totalPixels();
    }

    void Client::endFrame()
    {
      DW_DBG(printf("#osp.dw(dsp): client %i/%i barriering on %i/%i\n",me.rank,me.size,
                 displayGroup.rank,displayGroup.size));
      MPI_CALL(Barrier(displayGroup.comm));
    }

    __thread void *g_compressor = NULL;

    void Client::writeTile(const PlainTile &tile)
    {
      assert(wallConfig);

      CompressedTile encoded;
#if 1
      if (!g_compressor) g_compressor = CompressedTile::createCompressor();
      void *compressor = g_compressor;
      encoded.encode(compressor,tile);
#else
      void *compressor = CompressedTile::createCompressor();
      encoded.encode(compressor,tile);
      CompressedTile::freeCompressor(compressor);
#endif

      // -------------------------------------------------------
      // compute displays affected by this tile
      // -------------------------------------------------------
      const box2i affectedDisplays = wallConfig->affectedDisplays(tile.region);

      // -------------------------------------------------------
      // now, send to all affected displays ...
      // -------------------------------------------------------

      DW_DBG(static std::atomic<int> numSent;
             numSent += tile.region.size().product();
             printf("region %i %i - %i %i displays %i %i - %i %i : %i\n",
                    tile.region.lower.x,
                    tile.region.lower.y,
                    tile.region.upper.x,
                    tile.region.upper.y,
                    affectedDisplays.lower.x,
                    affectedDisplays.lower.y,
                    affectedDisplays.upper.x,
                    affectedDisplays.upper.y,
                    (int)numSent);
             usleep(1000));

      for (int dy=affectedDisplays.lower.y;dy<affectedDisplays.upper.y;dy++)
        for (int dx=affectedDisplays.lower.x;dx<affectedDisplays.upper.x;dx++) 
          encoded.sendTo(displayGroup,wallConfig->rankOfDisplay(vec2i(dx,dy)));
    }

  } // ::ospray::dw
} // ::ospray
