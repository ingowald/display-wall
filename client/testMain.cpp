#include "../common/MPI.h"
#include "../common/WallConfig.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;
    
    using std::cout; 
    using std::endl;
    using std::flush;

    std::string portNameFile = ".ospDisplayWald.port";
    
    std::string readPortName()
    {
      FILE *file = fopen(portNameFile.c_str(),"r");
      if (!file) 
        throw std::runtime_error("could not open port name file '"+portNameFile+"'");
      char line[1000];
      fgets(line,1000,file);
      char *eol = strstr(line,"\n");
      if (eol) *eol = 0;
      fclose(file);
      return line;
    }

    WallConfig receiveDisplayConfig(const MPI::Group &display,
                                    const MPI::Group &me)
    {
      vec2i numDisplays;
      vec2i pixelsPerDisplay;
      if (me.rank == 0) 
        cout << "waiting for service to tell us the display wall config..." << endl;
      MPI_CALL(Bcast(&numDisplays,sizeof(numDisplays),MPI_BYTE,0,display.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,sizeof(pixelsPerDisplay),MPI_BYTE,0,display.comm));
      return WallConfig(numDisplays,pixelsPerDisplay);
    }

    /*! establish connection between 'me' and the remote service */
    MPI::Group establishConnection(std::string portName,
                                   const MPI::Group &me)
    {
      me.barrier();
      MPI::Group toDisplay;
      if (me.rank == 0) {
        if (portName.empty())
          portName = readPortName();
        cout << "trying to connect to port '"<<portName << "'" << endl;
      }
      MPI_CALL(Comm_connect(portName.c_str(),MPI_INFO_NULL,0,me.comm,&toDisplay.comm));
      toDisplay = MPI::Group(toDisplay.comm);

      if (me.rank == 0)
        cout << "connection established..." << endl;
      me.barrier();
      return toDisplay;
    }

    void sendTile(const MPI::Group &display,
                  const WallConfig &wallConfig,
                  const vec2i &begin, 
                  const vec2i &end, 
                  int pitch,
                  const uint32 *pixel)
    {
      int numInts = 4+(end-begin).product();
      int *buffer = new int[numInts];
      int *write = buffer;
      *write++ = begin.x;
      *write++ = begin.y;
      *write++ = end.x;
      *write++ = end.y;

      const int *line = (const int *)pixel;
      for (int iy=begin.y;iy<end.y;iy++) {
        const int *in = line;
        for (int ix=begin.x;ix<end.x;ix++)
          *write++ = *line++;
        line += pitch;
      }

      // -------------------------------------------------------
      // compute displays affected by this tile
      // -------------------------------------------------------
      vec2i affectedDisplay_begin = begin / wallConfig.pixelsPerDisplay;
      vec2i affectedDisplay_end = divRoundUp(end,wallConfig.pixelsPerDisplay);

      // -------------------------------------------------------
      // now, send to all affected displays ...
      // -------------------------------------------------------
      for (int dy=affectedDisplay_begin.y;dy<affectedDisplay_begin.y;dy++)
        for (int dx=affectedDisplay_begin.x;dx<affectedDisplay_begin.x;dx++) {
          MPI_CALL(Send(buffer,numInts*sizeof(int),MPI_BYTE,0,0,display.comm));
        }
      delete [] buffer;
    }

    void renderFrame(const WallConfig &wallConfig, 
                     const MPI::Group &display,
                     const MPI::Group &me)
    {
      static int frameID = 0;

      vec2i tileSize(160,10);
      vec2i numTiles = divRoundUp(wallConfig.totalPixels(),tileSize);
      for (int iy=0;iy<numTiles.y;iy++)
        for (int ix=0;ix<numTiles.x;ix++) {
          int tileID = ix + numTiles.x * iy;
          if ((tileID % me.size) != me.rank)
            continue;
          vec2i lo = vec2i(ix,iy)*tileSize;
          vec2i hi = min(lo+tileSize,wallConfig.totalPixels());
          uint32 *pixel = new uint32[tileSize.x*tileSize.y];
          sendTile(display,wallConfig,lo,hi,tileSize.x,pixel);
          delete[] pixel;
        }
      
      ++frameID;
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
      MPI::Group displayGroup = establishConnection(portName,me);
      WallConfig wallConfig = receiveDisplayConfig(displayGroup,me);
      if (me.rank == 0)
        wallConfig.print();

      while (1)
        renderFrame(wallConfig,displayGroup,me);

      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
