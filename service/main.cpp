#include "Server.h"
#include "GlutWindow.h"
//#include "ospray/common/Thread.h"
#include <thread>

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    // default settings
    bool hasHeadNode = false;
    bool doStereo    = false;
    WallConfig::DisplayArrangement arrangement = WallConfig::Arrangement_xy;
    
    vec2i windowSize(320,240);
    vec2i numDisplays(0,0);

    void usage(const std::string &err)
    {
      if (!err.empty()) {
        cout << "Error: " << err << endl << endl;
      }
      cout << "usage: ./ospDisplayWald [args]*" << endl << endl;
      cout << "w/ args: " << endl;
      cout << "--width|-w <numDisplays.x>     - num displays in x direction" << endl;
      cout << "--height|-h <numDisplays.y>    - num displays in y direction" << endl;
      cout << "--[no-]head-node | -[n]hn      - use / do not use dedicated head node" << endl;
      exit(!err.empty());
    }

    /*! the display callback */
    void displayNewFrame(const uint32_t *left, 
                         const uint32_t *right, 
                         void *object)
    {
      GlutWindow *window = (GlutWindow*)object;
      window->setFrameBuffer(left,right);
    }

    extern "C" int main(int ac, char **av)
    {
      glutInit(&ac, (char **) av);
      MPI::init(ac,av);
      MPI::Group world(MPI_COMM_WORLD);

      for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        if (arg == "--head-node" || arg == "-hn") {
          hasHeadNode = true;
        } else if (arg == "--no-head-node" || arg == "-nhn") {
          hasHeadNode = false;
        } else if (arg == "--width" || arg == "-w") {
          numDisplays.x = atoi(av[++i]);
        } else if (arg == "--height" || arg == "-h") {
          numDisplays.y = atoi(av[++i]);
        } else {
          usage("unkonwn arg "+arg);
        } 
      }

      if (numDisplays.x < 1) 
        usage("no display wall width specified (--width <w>)");
      if (numDisplays.y < 1) 
        usage("no display wall height specified (--heigh <h>)");
      if (world.size != numDisplays.x*numDisplays.y+hasHeadNode)
        throw std::runtime_error("invalid number of ranks for given display/head node config");
      const char *title = "display wall window";
      PING;
      GlutWindow glutWindow(windowSize,title);

      PING;
      WallConfig wallConfig(numDisplays,windowSize,
                            arrangement,doStereo);


      PING;
      if (hasHeadNode && world.rank == 0) {
        cout << "running a dedicated headnode on rank 0; "
             << "not creating a window there" << endl;
      } else {
        glutWindow.create();
      }
      
      PING;
      startDisplayWallService(world.comm,wallConfig,hasHeadNode,
                              displayNewFrame,&glutWindow);

      // std::thread commThread([&]() {
      //     setupCommunications(&glutWindow,wallConfig,hasHeadNode,world);
      //   });

      PING;
      sleep(1);
      PING;
      if (hasHeadNode && world.rank == 0) {
        /* no window on head node */
        PING;
        throw std::runtime_error("should never reach this ...");
      } else {
        glutWindow.run();
      }
      PING;
      // commThread.join();
      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
