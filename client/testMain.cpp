#include "../common/MPI.h"
#include "common/vec.h"

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
      PRINT(line);
      fclose(file);
      return line;
    }

    void receiveDisplayConfig(const MPI::Group &display,
                              const MPI::Group &me)
    {
      vec2i numDisplays;
      vec2i pixelsPerDisplay;
      if (me.rank == 0) 
        cout << "waiting for service to tell us the display wall config..." << endl;
      MPI_CALL(Bcast(&numDisplays,sizeof(numDisplays),MPI_BYTE,0,display.comm));
      MPI_CALL(Bcast(&pixelsPerDisplay,sizeof(pixelsPerDisplay),MPI_BYTE,0,display.comm));
      if (me.rank == 0) {
        PRINT(numDisplays);
        PRINT(pixelsPerDisplay);
      }
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

    void renderFrame()
    {
      throw std::runtime_error("not implemented");
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
      receiveDisplayConfig(displayGroup,me);

      while (1)
        renderFrame();

      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
