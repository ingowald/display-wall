#include "../common/MPI.h"
#include "../common/WallConfig.h"
#include "../common/CompressedTile.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    struct Client {
      Client(const MPI::Group &me,
             const std::string &portName);
      void receiveDisplayConfig();
      /*! establish connection between 'me' and the remote service */
      void establishConnection(std::string portName);
      /*! return total pixels in display wall, so renderer/app can
          know how large a frame buffer to use ... */
      vec2i totalPixelsInWall() const;
      void writeTile(const PlainTile &tile);

      WallConfig *wallConfig;
      MPI::Group displayGroup;
      MPI::Group me;
    };

  } // ::ospray::dw
} // ::ospray
