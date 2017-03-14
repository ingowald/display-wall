/* 
Copyright (c) 2016-2017 Ingo Wald

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

#pragma once

#include "FrameBuffer.h"
// windowing stuff
#include "GLFW/glfw3.h"
// std
#include <mutex>
#include <condition_variable>

namespace ospray {
  namespace dw {

    struct GLFWindow 
    {
      GLFWindow(const vec2i &size, const vec2i &position, const std::string &title,
                 bool doFullScreen, bool stereo=false);

      virtual ~GLFWindow()
      {
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        PING;
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
      }

      void setFrameBuffer(const uint32_t *leftEye,
                          const uint32_t *rightEye);
      void display(); 
      vec2i getSize()   const;
      bool doesStereo() const;
      void run();
      void create();

      static vec2i getScreenSize() 
      {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return vec2i(mode->width,mode->height);
      }

    // private:
      const uint32_t *leftEye;
      const uint32_t *rightEye;

      GLFWwindow *handle { nullptr };

      std::mutex mutex;
      std::condition_variable newFrameAvail;
      std::condition_variable newFrameDisplayed;

      vec2i size, position;
      bool stereo;
      int receivedFrameID;
      int displayedFrameID;
      bool doFullScreen;
      std::string title;
      // static GLFWindow *singleton;
    };
    
  } // ::ospray::dw
} // ::ospray
