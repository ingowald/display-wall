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
      int arrangement;
      int stereo;
      if (me.rank == 0) 
        cout << "waiting for service to tell us the display wall config..." << endl;
      MPI_CALL(Bcast(&numDisplays,2,MPI_INT,0,displayGroup.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,2,MPI_INT,0,displayGroup.comm));
      MPI_CALL(Bcast(&arrangement,1,MPI_INT,0,displayGroup.comm));
      MPI_CALL(Bcast(&stereo,1,MPI_INT,0,displayGroup.comm));
      wallConfig = new WallConfig(numDisplays,pixelsPerDisplay,
                                  (WallConfig::DisplayArrangement)arrangement,
                                  stereo);
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

    vec2i Client::totalPixelsInWall() const 
    {
      assert(wallConfig);
      return wallConfig->totalPixels();
    }

    void Client::writeTile(const PlainTile &tile)
    {
      assert(wallConfig);

      CompressedTile encoded;
      encoded.encode(tile);

      printf("#dw.c: writing (%i,%i)-(%i,%i)\n",
             tile.region.lower.x,
             tile.region.lower.y,
             tile.region.upper.x,
             tile.region.upper.y);
      // -------------------------------------------------------
      // compute displays affected by this tile
      // -------------------------------------------------------
      vec2i affectedDisplay_begin = tile.region.lower / wallConfig->pixelsPerDisplay;
      vec2i affectedDisplay_end = divRoundUp(tile.region.upper,
                                             wallConfig->pixelsPerDisplay);

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
