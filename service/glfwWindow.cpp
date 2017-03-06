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

#include "glfwWindow.h"

namespace ospray {
  namespace dw {

    using std::endl;
    using std::cout;

    GLFWindow::GLFWindow(const vec2i &size,
                         const vec2i &position,
                         const std::string &title,
                         bool doFullScreen, 
                         bool stereo)
      : size(size),
        position(position),
        title(title),
        leftEye(NULL),
        rightEye(NULL),
        stereo(stereo),
        receivedFrameID(-1),
        displayedFrameID(-1),
        doFullScreen(doFullScreen)
    {
      PING;
      if (!glfwInit())
        {
          fprintf(stderr, "Failed to initialize GLFW\n");
          exit(EXIT_FAILURE);
        }
      create();
    }


    void GLFWindow::create()
    {
      PING;
      if (singleton != NULL)
        throw std::runtime_error("can only have one active GLFWindow right now ....");
      else 
        singleton = this;
        
//       if (stereo) {
// #if DBG_FAKE_STEREO
//         std::cout << "WARNING: Faking stereo for now - this will NOT work for real stereo devices" << std::endl;
//         glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
// #else
//         glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO);
// #endif
//       } else
        // glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

      window = glfwCreateWindow(size.x,size.y,title.c_str(),
                                NULL,NULL);
        
      // glutInitWindowSize( size.x, size.y );
      // glutInitWindowPosition( position.x, position.y );
      // windowID = glutCreateWindow(title.c_str());
      // glutDisplayFunc(glutDisplay);
      // glutIdleFunc(glutIdle);
      if (doFullScreen) {
        throw std::runtime_error("fullscreen not yet implemented");
        // glutFullScreen();
      }

      // glfwSetWindowRefreshCallback(window,refresh);
      glfwShowWindow(window);
    }

    void GLFWindow::setFrameBuffer(const uint32_t *leftEye, const uint32 *rightEye)
    {
      {
        std::lock_guard<std::mutex> lock(mutex);
        this->leftEye = leftEye;
        this->rightEye = rightEye;
        receivedFrameID++;
        newFrameAvail.notify_one();
      }
#if 0
      {
        std::unique_lock<std::mutex> lock(mutex);
        // cout << "waiting for display (" << displayedFrameID << ")" << endl;
        newFrameDisplayed.wait(lock, [&]{
            // cout << "waiting for display (" << displayedFrameID << ")" << endl;
            return displayedFrameID==receivedFrameID;
          });
      }
#endif
    }

    // void GLFWindow::glutIdle() 
    // { 
    //   glutPostRedisplay(); 
    // }

    void GLFWindow::display() 
    {
      {
        std::unique_lock<std::mutex> lock(mutex);
        newFrameAvail.wait(lock,[this](){return receivedFrameID > displayedFrameID; });
        
        if (!leftEye) {
          // printf("no frame buffer set, yet\n");
        } else if (stereo) {
          // with stereo drawing ...
#if DBG_FAKE_STEREO
          static int frameID = 0;
          if ((++frameID) % 2) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, leftEye);
          } else {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, rightEye);
          }
#else
          glDrawBuffer(GL_BACK_LEFT);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, leftEye);
        
          assert(rightEye != NULL);
          glDrawBuffer(GL_BACK_RIGHT); 
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, rightEye);
#endif
        } else {
          assert(rightEye == NULL);
          // no stereo
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, leftEye);
        }
      
        // glutSwapBuffers();
      }
      {
        std::lock_guard<std::mutex> lock(mutex);
        displayedFrameID++;
        newFrameDisplayed.notify_one();
      }
    }

    // void GLFWindow::glutDisplay() 
    // {
    //   assert(singleton);
    //   singleton->display();
    // }

    vec2i GLFWindow::getSize() const 
    { 
      return size; 
    }

    bool GLFWindow::doesStereo() const
    { 
      return stereo; 
    }
    
    void GLFWindow::run() 
    { 
      PING;
      PRINT(this);
      PRINT(window);
      while (!glfwWindowShouldClose(window)) {
        PING;

        PING;

        glfwSwapBuffers(window);
        glfwPollEvents();
        usleep(10000);
      }
    }
    
    GLFWindow *GLFWindow::singleton = NULL;
    
  } // ::ospray::dw
} // ::ospray
