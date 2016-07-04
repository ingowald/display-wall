#pragma once

#include "../common/MPI.h"
#include "../common/WallConfig.h"
#include <thread>

namespace ospray {
  namespace dw {

    #define DW_STEREO 1
    #define DW_HAVE_HEAD_NODE 2
    
    typedef void (*DisplayCallback)(const uint32_t *leftEye, 
                                    const uint32_t *rightEye,
                                    void *objects);

    void startDisplayWallService(const MPI_Comm comm,
                                 const WallConfig &wallConfig,
                                 bool hasHeadNode,
                                 DisplayCallback displayCallback,
                                 void *objectForCallback);

  } // ::ospray::dw
} // ::ospray
