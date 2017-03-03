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

#include "Client.h"
#include "ospcommon/tasking/parallel_for.h"
// std
#include <vector>

namespace ospray {
  namespace dw {

    using namespace ospcommon;
    
    using std::cout; 
    using std::endl;
    using std::flush;

    void renderFrame(const MPI::Group &me, Client *client)
    {
      static size_t frameID = 0;
      static double lastTime = getSysTime();

      assert(client);
      const vec2i totalPixels = client->totalPixelsInWall();
      vec2i tileSize(32);
      // vec2i tileSize(160,10);
      vec2i numTiles = divRoundUp(totalPixels,tileSize);
      size_t tileCount = numTiles.product();
      parallel_for(tileCount,[&](int tileID){
          if ((tileID % me.size) != me.rank)
            return;

          PlainTile tile(tileSize);
          const int tile_x = tileID % numTiles.x;
          const int tile_y = tileID / numTiles.x;
          
          tile.region.lower = vec2i(tile_x,tile_y)*tileSize;
          tile.region.upper = min(tile.region.lower+tileSize,totalPixels);

          for (int iy=tile.region.lower.y;iy<tile.region.upper.y;iy++)
            for (int ix=tile.region.lower.x;ix<tile.region.upper.x;ix++) {
              int r = (frameID+ix) % 255;
              int g = (frameID+iy) % 255;
              int b = (frameID+ix+iy) % 255;
              int rgba = (b<<16)+(g<<8)+(r<<0);
              tile.pixel[(ix-tile.region.lower.x)+tileSize.x*(iy-tile.region.lower.y)] = rgba;
            }

          assert(client);
          client->writeTile(tile);
        });
      ++frameID;
      double thisTime = getSysTime();
      printf("done rendering frame %li (%f fps)\n",frameID,1.f/(thisTime-lastTime));
      lastTime = thisTime;
      // me.barrier();
      client->endFrame();
    }

    extern "C" int main(int ac, char **av)
    {
      MPI::init(ac,av);
      MPI::Group world(MPI_COMM_WORLD);

      std::string portName = "";

      std::vector<std::string> nonDashArgs;
      for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        if (arg[0] == '-') {
          throw std::runtime_error("unknown arg "+arg);
        } else
          nonDashArgs.push_back(arg);
      }

      if (nonDashArgs.size() != 2) {
        cout << "Usage: ./ospDwTest <hostName> <portNo>" << endl;
        exit(1);
      }
      const std::string hostName = nonDashArgs[0];
      const int portNum = atoi(nonDashArgs[1].c_str());

      ServiceInfo serviceInfo;
      serviceInfo.getFrom(hostName,portNum);

      // -------------------------------------------------------
      // args parsed, now do the job
      // -------------------------------------------------------
      MPI::Group me = world.dup();
      Client *client = new Client(me,serviceInfo.mpiPortName);

      while (1)
        renderFrame(me,client);

      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
