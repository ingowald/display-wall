/* 
   Copyright (c) 2016 Ingo Wald

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

#include "GlutWindow.h"

namespace ospray {
  namespace dw {

    using std::endl;
    using std::cout;

    GlutWindow::GlutWindow(const vec2i &size, 
                           const std::string &title, 
                           bool stereo)
      : size(size),
        title(title),
        windowID(-1),
        leftEye(NULL),
        rightEye(NULL),
        stereo(stereo),
        receivedFrameID(-1),
        displayedFrameID(-1)
    {
    }

    void GlutWindow::create()
    {
      if (singleton != NULL)
        throw std::runtime_error("can only have one active GlutWindow right now ....");
      else 
        singleton = this;
        
      if (stereo) 
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO);
      else
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
        
      glutInitWindowSize( size.x, size.y );
      windowID = glutCreateWindow(title.c_str());
      glutDisplayFunc(glutDisplay);
      glutIdleFunc(glutIdle);
    }

    void GlutWindow::setFrameBuffer(const uint32_t *leftEye, const uint32 *rightEye)
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

    void GlutWindow::glutIdle() 
    { 
      glutPostRedisplay(); 
    }

    void GlutWindow::display() 
    {
      {
        std::unique_lock<std::mutex> lock(mutex);
        newFrameAvail.wait(lock,[this](){return receivedFrameID > displayedFrameID; });
        
        if (!leftEye) {
          // printf("no frame buffer set, yet\n");
        } else if (stereo) {
          // with stereo drawing ...
          glDrawBuffer(GL_BACK_LEFT);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, leftEye);
        
          assert(rightEye != NULL);
          glDrawBuffer(GL_BACK_RIGHT); 
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, rightEye);
        } else {
          assert(rightEye == NULL);
          // no stereo
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, leftEye);
        }
      
        glutSwapBuffers();
      }
      {
        std::lock_guard<std::mutex> lock(mutex);
        displayedFrameID++;
        newFrameDisplayed.notify_one();
      }
    }

    void GlutWindow::glutDisplay() 
    {
      assert(singleton);
      singleton->display();
    }

    vec2i GlutWindow::getSize() const 
    { 
      return size; 
    }

    bool GlutWindow::doesStereo() const
    { 
      return stereo; 
    }
    
    void GlutWindow::run() 
    { 
      glutMainLoop(); 
    }
    
    GlutWindow *GlutWindow::singleton = NULL;
    
  } // ::ospray::dw
} // ::ospray
