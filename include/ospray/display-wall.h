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

#pragma once

/*! opaque type for a handle to a display wall service */
typedef struct _OSPDisplayWall *OSPDisplayWall;

/*! structure that the display wall uses to tell us what its configuration is */
struct OSPDisplayWallInfo {
  int32_t totalPixels_x;
  int32_t totalPixels_y;
  int32_t stereo;
  char    mpiPortName[MPI_MAX_PORT_NAME];
};

#define OSPRAY_DW_API extern "C"


/*! initialize a new display wall session; the display wall will be
    looked for at the given mpi service port name, and a connection
    will be made (if service is found) by the given mpi group. if
    successful a valid handle gets returned; if not, we return NULL */
OSPRAY_DW_API OSPDisplayWall ospDwInit(MPI_Comm comm,
                                       const char *portName);

/*! query information on the display wall, from service running at
  given hostname:portnum. if successful the 'infoResult' will be set
  to the given service's configuration. if not, totalPixel will be
  set to 0:0 */
OSPRAY_DW_API void OSPDwGetInfo(OSPDisplayWallInfo *infoResult,
                                const char *hostName,
                                const int portNum);

/*! end latest frame; all workers have to call this function
  collaboratively once at the end of each frame */
OSPRAY_DW_API void ospDwEndFrame(OSPDisplayWall dw);

/*! write given tile of pixels to the display wall. pixel[] holds the
    pixels, and is an array of size_x * size_y 32-bit pixels. note
    that this tile MAY extend beyond the actually valid size of the
    display wall. */
OSPRAY_DW_API void ospDwWriteTile(OSPDisplayWall dw,
                                  int eye /*!< 0 for left eye, 1 for
                                             right; if non-stereo use 0 */,
                                  int32_t where_x, int32_t where_y,
                                  int32_t size_x,  int32_t size_y,
                                  const uint32_t *pixel);

