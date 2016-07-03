
#include "../common/MPI.h"
#include "../common/CompressedTile.h"
#include "../common/WallConfig.h"
#include "GlutWindow.h"
//#include "ospray/common/Thread.h"
#include <thread>

namespace ospray {
  namespace dw {

    using std::cout; 
    using std::endl;
    using std::flush;

#if 0
    struct TileWriter {
      TileWriter(GlutWindow *window);

      FrameBuffer *writeFB, *displayFB;
      GlutWindow  *window;
      size_t       numPixelsWrittenThisFrame;
      size_t       totalPixelsInFrame;
      std::mutex   mutex;
    };
#endif

    void processIncomingTiles(MPI::Group &outside,MPI::Group &me)
    {
      cout << "running tile receiver ..." << endl;
      while (1) {
        // -------------------------------------------------------
        // receive one tiles
        // -------------------------------------------------------
        CompressedTile encoded;
        encoded.receiveOne(outside);

        PlainTile plain;
        encoded.decode(plain);

        
        throw std::runtime_error("need to decode tile here ... ");
      }
    }

  } // ::ospray::dw
} // ::ospray
