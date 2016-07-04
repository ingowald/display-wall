#pragma once

#include "FrameBuffer.h"
// opengl for windowing stuff
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
// std
#include <mutex>
#include <condition_variable>

namespace ospray {
  namespace dw {

    struct GlutWindow 
    {
      GlutWindow(const vec2i &size, const std::string &title, bool stereo=false);
      void setFrameBuffer(const uint32_t *leftEye,
                          const uint32_t *rightEye);
      void display(); 
      vec2i getSize()   const;
      bool doesStereo() const;
      void run();
      void create();
      
      static void glutIdle();
      static void glutDisplay();
      
    private:
      const uint32_t *leftEye;
      const uint32_t *rightEye;
      std::mutex mutex;
      std::condition_variable newFrameAvail;

      int windowID;
      vec2i size;
      std::string title;
      bool stereo;
      int receivedFrameID;
      int displayedFrameID;
      static GlutWindow *singleton;
    };
    
  } // ::ospray::dw
} // ::ospray
