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

// ospray pixel 
#include "ospray/fb/PixelOp.h"
#include "ospray/fb/FrameBuffer.h"
#include "ospray/mpi/MPICommon.h"
// displaywald client
#include "../client/Client.h"

namespace ospray {
  namespace dw {

    void foo()
    {
    }

    struct DisplayWaldPixelOp : public ospray::PixelOp 
    {
      struct Instance : public ospray::PixelOp::Instance 
      {
        Instance(FrameBuffer *fb, 
                 PixelOp::Instance *prev,
                 dw::Client *client)
          : client(client)
        {
          fb->pixelOp = this;
        }

        // /*! gets called every time the frame buffer got 'commit'ted */
        // virtual void  commitNotify() {}
        // /*! gets called once at the end of the frame */
        virtual void endFrame() 
        { client->endFrame(); }
      
        /*! called right after the tile got accumulated; i.e., the
          tile's RGBA values already contain the accu-buffer blended
          values (assuming an accubuffer exists), and this function
          defines how these pixels are being processed before written
          into the color buffer */
        virtual void postAccum(Tile &tile) 
        {
          PlainTile plainTile(vec2i(TILE_SIZE));
          plainTile.pitch = TILE_SIZE;
          for (int i=0;i<TILE_SIZE*TILE_SIZE;i++) {
            int r = std::min(255,int(255.f*tile.r[i]));
            int g = std::min(255,int(255.f*tile.g[i]));
            int b = std::min(255,int(255.f*tile.b[i]));
            int rgba = (b<<24)|(g<<16)|(r<<8);
            plainTile.pixel[i] = rgba;
          }
          plainTile.region = tile.region;
          bool stereo = client->getWallConfig()->doStereo();
          if (!stereo) {
            plainTile.eye = 0;
            client->writeTile(plainTile);
          } else {
            int trueScreenWidth = client->getWallConfig()->totalPixels().x;
            if (plainTile.region.upper.x <= trueScreenWidth) {
              // all on left eye
              plainTile.eye = 0;
              client->writeTile(plainTile);
            } else if (plainTile.region.lower.x >= trueScreenWidth) {
              // all on right eye - just shift coordinates
              plainTile.region.lower.x -= trueScreenWidth;
              plainTile.region.upper.x -= trueScreenWidth;
              plainTile.eye = 1;
              client->writeTile(plainTile);
            } else {
              // overlaps both sides - split it up
              const int original_lower_x = plainTile.region.lower.x;
              const int original_upper_x = plainTile.region.upper.x;
              // first, 'clip' the tile and write 'clipped' one to the left side
              plainTile.region.lower.x = original_lower_x - trueScreenWidth;
              plainTile.region.upper.x = trueScreenWidth;
              plainTile.eye = 0;
              client->writeTile(plainTile);

              // now, move right true to the left, clip on lower side, and shift pixels
              plainTile.region.lower.x = 0;
              plainTile.region.upper.x = original_upper_x - trueScreenWidth;
              // since pixels didn't start at 'trueWidth' but at 'lower.x', we have to shift
              int numPixelsInRegion = plainTile.region.size().y*plainTile.pitch;
              int shift = trueScreenWidth - original_lower_x;
              plainTile.eye = 1;
              for (int i=0;i<numPixelsInRegion;i++)
                plainTile.pixel[i] = plainTile.pixel[i+shift];
              client->writeTile(plainTile);
            }
              
          }
        }
 
        //! \brief common function to help printf-debugging 
        /*! Every derived class should overrride this! */
        virtual std::string toString() const;

        dw::Client *client;
      };
      
      //! \brief common function to help printf-debugging 
      /*! Every derived class should overrride this! */
      virtual std::string toString() const;
      
      /*! \brief commit the object's outstanding changes (such as changed
       *         parameters etc) */
      virtual void commit()
      {
        std::string streamName = getParamString("streamName","");
        std::cout << "#osp:dw: trying to establish connection to display wall service at MPI port " << streamName << std::endl;
        client = new dw::Client(mpi::worker.comm,streamName);
      }

      //! \brief create an instance of this pixel op
      virtual ospray::PixelOp::Instance *createInstance(FrameBuffer *fb, 
                                                        PixelOp::Instance *prev)
      {
        return new Instance(fb,prev,client);
      }
      
      dw::Client *client;
    };
    
    //! \brief common function to help printf-debugging 
    std::string DisplayWaldPixelOp::toString() const 
    { return "ospray::dw::DisplayWaldPixelOp (displayWald module)"; }

    //! \brief common function to help printf-debugging 
    std::string DisplayWaldPixelOp::Instance::toString() const 
    { return "ospray::dw::DisplayWaldPixelOp::Instance (displayWald module)"; }

    extern "C" void ospray_init_module_displayWald()
    {
      printf("loading the 'displayWald' module...\n");
    }

    OSP_REGISTER_PIXEL_OP(DisplayWaldPixelOp,display_wald);

  } // ::ospray::dw
} // ::ospray
