
#include "Server.h"
#include "../common/CompressedTile.h"

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

    /*! the code that actually receives the tiles, decompresses
      them, and writes them into the current (write-)frame buffer */
    void Server::processIncomingTiles(MPI::Group &outside)
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
