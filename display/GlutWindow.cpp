#include "GlutWindow.h"

namespace ospray {
  namespace dw {

    GlutWindow::GlutWindow(const vec2i &size, 
                           const std::string &title, 
                           bool stereo)
      : size(size),
        title(title),
        windowID(-1),
        fb(NULL),
        stereo(stereo)
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

    void GlutWindow::setFrameBuffer(FrameBuffer *fb)
    {
      mutex.lock();
      this->fb = fb;
      mutex.unlock();
    }

    void GlutWindow::glutIdle() 
    { 
      usleep(1000); 
      glutPostRedisplay(); 
    }

    void GlutWindow::display() 
    {
      static int frameID = 0;
      mutex.lock();
      printf("displaying frame %i\n",++frameID);
      mutex.unlock();

      if (!fb) {
        // printf("no frame buffer set, yet\n");
      } else if (stereo) {
        // with stereo drawing ...
        glDrawBuffer(GL_BACK_LEFT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, fb->pixels[0]);

        assert(fb->pixel[1]);
        glDrawBuffer(GL_BACK_RIGHT); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, fb->pixels[1]);
      } else {
        assert(fb->pixel[1] == NULL);
        // no stereo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawPixels(size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, fb->pixels[0]);
      }

      glutSwapBuffers();
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
