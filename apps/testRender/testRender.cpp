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

//#include "../../client/Client.h"
#include "../common/MPI.h"
#include "ospray/display-wall.h"

#include "ospray/common/tasking/parallel_for.h"
// std
#include <vector>

namespace ospray {
  namespace dw {

    using namespace ospcommon;
    
    using std::cout; 
    using std::endl;
    using std::flush;

    OSPDisplayWallInfo dwInfo;
    
    void renderFrame(const MPI::Group &me, OSPDisplayWall dw
                     // Client *client
                     )
    {
      static size_t frameID = 0;
      static double lastTime = getSysTime();

      // assert(client);
      assert(dw);
      
      const vec2i totalPixels(dwInfo.totalPixels_x,dwInfo.totalPixels_y);
      // const vec2i totalPixels = client->totalPixelsInWall();
      vec2i tileSize(32);
      // vec2i tileSize(160,10);
      vec2i numTiles = divRoundUp(totalPixels,tileSize);
      size_t tileCount = numTiles.product();
      parallel_for(tileCount,[&](int tileID){
          if ((tileID % me.size) != me.rank)
            return;

          const int tile_x = tileID % numTiles.x;
          const int tile_y = tileID / numTiles.x;

          // tile coordinates
          const vec2i lower = vec2i(tile_x,tile_y)*tileSize;
          const vec2i upper = min(lower+tileSize,totalPixels);
          uint32_t pixel[tileSize.x*tileSize.y];
          // clear the tile in case it extends beyond the valid range
          for (int i=0;i<tileSize.x*tileSize.y;i++)
            pixel[i] = 0;
          for (int iy=lower.y;iy<upper.y;iy++)
            for (int ix=lower.x;ix<upper.x;ix++) {
              int r = (frameID+ix) % 255;
              int g = (frameID+iy) % 255;
              int b = (frameID+ix+iy) % 255;
              int rgba = (b<<16)+(g<<8)+(r<<0);
              pixel[(ix-lower.x)+tileSize.x*(iy-lower.y)] = rgba;
            }

          // assert(client);
          assert(dw);
          // client->writeTile(tile);
          ospDwWriteTile(dw,0,lower.x,lower.y,tileSize.x,tileSize.y,pixel);
        });
      ++frameID;
      double thisTime = getSysTime();
      printf("done rendering frame %li (%f fps)\n",frameID,1.f/(thisTime-lastTime));
      lastTime = thisTime;
      // me.barrier();
      // client->endFrame();
      ospDwEndFrame(dw);
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

      // ServiceInfo dwInfo;
      // dwInfo.getFrom(hostName,portNum);
      OSPDwGetInfo(&dwInfo,hostName.c_str(),portNum);
      if (dwInfo.totalPixels_x <= 0)
        throw std::runtime_error("could not get display wall configuration.");
      
      // -------------------------------------------------------
      // args parsed, now do the job
      // -------------------------------------------------------
      MPI::Group me = world.dup();
      // Client *client = new Client(me,dwInfo.mpiPortName);
      OSPDisplayWall dw = ospDwInit(me,dwInfo.mpiPortName);
      
      while (1)
        renderFrame(me,dw);

      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
