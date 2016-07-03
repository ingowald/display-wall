#include "../common/MPI.h"
#include "../common/CompressedTile.h"
#include "../common/WallConfig.h"

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

    /*! the dispatcher that receives tiles on the head node, and then
        dispatches them to the actual tile receivers */
    void runDispatcher(const MPI::Group &outsideClients,
                       const MPI::Group &displayGroup,
                       const WallConfig &wallConfig)
    {
      // std::thread *dispatcherThread = new std::thread([=]() {
      std::cout << "Running the dispatcher thread ..." << std::endl;
      while (1) {
        CompressedTile encoded;
        encoded.receiveOne(outsideClients);

        const box2i region = encoded.getRegion();
        
        // -------------------------------------------------------
        // compute displays affected by this tile
        // -------------------------------------------------------
        vec2i affectedDisplay_begin = region.lower / wallConfig.pixelsPerDisplay;
        vec2i affectedDisplay_end = divRoundUp(region.upper,wallConfig.pixelsPerDisplay);
        
        // -------------------------------------------------------
        // now, send to all affected displays ...
        // -------------------------------------------------------
        for (int dy=affectedDisplay_begin.y;dy<affectedDisplay_end.y;dy++)
          for (int dx=affectedDisplay_begin.x;dx<affectedDisplay_end.x;dx++) 
            encoded.sendTo(displayGroup,wallConfig.rankOfDisplay(vec2i(dx,dy)));;
        
      };
      // });
    }

    
  } // ::ospray::dw
} // ::ospray
