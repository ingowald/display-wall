#include "Client.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;
    
    using std::cout; 
    using std::endl;
    using std::flush;

    void renderFrame(const MPI::Group &me, Client *client)
    {
      assert(client);
      const vec2i totalPixels = client->totalPixelsInWall();
      vec2i tileSize(160,10);
      vec2i numTiles = divRoundUp(totalPixels,tileSize);
      PlainTile tile;
      for (int iy=0;iy<numTiles.y;iy++)
        for (int ix=0;ix<numTiles.x;ix++) {
          int tileID = ix + numTiles.x * iy;
          if ((tileID % me.size) != me.rank)
            continue;
          tile.region.lower = vec2i(ix,iy)*tileSize;
          tile.region.upper = min(tile.region.lower+tileSize,totalPixels);
          tile.pixel = new uint32_t[tileSize.x*tileSize.y];
          client->writeTile(tile);

          delete[] tile.pixel;
        }
    }

    extern "C" int main(int ac, char **av)
    {
      MPI::init(ac,av);
      MPI::Group world(MPI_COMM_WORLD);

      std::string portName = "";

      for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        // if (arg == "--head-node" || arg == "-hn") {
        //   hasHeadNode = true;
        // } else if (arg == "--no-head-node" || arg == "-nhn") {
        //   hasHeadNode = false;
        // } else {
        throw std::runtime_error("unkonwn arg "+arg);
        // } 
      }

      // -------------------------------------------------------
      // args parsed, now do the job
      // -------------------------------------------------------
      MPI::Group me = world.dup();
      Client *client = new Client(me,portName);

      while (1)
        renderFrame(me,client);

      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
