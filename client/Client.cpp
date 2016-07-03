#include "client.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;
        
    using std::cout; 
    using std::endl;
    using std::flush;


    std::string portNameFile = ".ospDisplayWald.port";
    
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

    void Client::receiveDisplayConfig() 
    {
      vec2i numDisplays;
      vec2i pixelsPerDisplay;
      if (me.rank == 0) 
        cout << "waiting for service to tell us the display wall config..." << endl;
      MPI_CALL(Bcast(&numDisplays,sizeof(numDisplays),MPI_BYTE,0,displayGroup.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,sizeof(pixelsPerDisplay),MPI_BYTE,0,displayGroup.comm));
      wallConfig = new WallConfig(numDisplays,pixelsPerDisplay);
    }

    /*! establish connection between 'me' and the remote service */
    void Client::establishConnection(std::string portName)
    {
      me.barrier();
      if (me.rank == 0) {
        if (portName.empty())
          portName = readPortName();
        cout << "trying to connect to port '"<<portName << "'" << endl;
      }
      MPI_Comm newComm;
      MPI_CALL(Comm_connect(portName.c_str(),MPI_INFO_NULL,0,me.comm,&newComm));
      this->displayGroup = MPI::Group(newComm);
      
      if (me.rank == 0)
        cout << "connection established..." << endl;
      me.barrier();
    }

    vec2i Client::totalPixelsInWall() const {
      assert(wallConfig);
      return wallConfig->totalPixels();
    }
    void Client::writeTile(const PlainTile &tile)
    {
      assert(wallConfig);

      CompressedTile encoded;
      encoded.encode(tile);

      // -------------------------------------------------------
      // compute displays affected by this tile
      // -------------------------------------------------------
      vec2i affectedDisplay_begin = tile.region.lower / wallConfig->pixelsPerDisplay;
      vec2i affectedDisplay_end = divRoundUp(tile.region.upper,wallConfig->pixelsPerDisplay);

      // -------------------------------------------------------
      // now, send to all affected displays ...
      // -------------------------------------------------------
      for (int dy=affectedDisplay_begin.y;dy<affectedDisplay_end.y;dy++)
        for (int dx=affectedDisplay_begin.x;dx<affectedDisplay_end.x;dx++) {
          encoded.sendTo(displayGroup,wallConfig->rankOfDisplay(vec2i(dx,dy)));
        }
    }


  } // ::ospray::dw
} // ::ospray
