## Copyright (c) 2016 Ingo Wald
## 
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
## 
## The above copyright notice and this permission notice shall be included in all
## copies or substantial portions of the Software.
## 
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.

OPTION(USE_TURBO_JPEG "Use Turbo-Jpeg compression?" OFF)

IF (USE_TURBO_JPEG) 
  ADD_DEFINITIONS(-DTURBO_JPEG=1)

  # find libjpeg-turbo
  FIND_PATH(TURBOJPEG_INCLUDE_DIR
    NAMES turbojpeg.h
    PATHS
    $ENV{LIBJPEG_TURBO_DIR}/include
    $ENV{TURBOJPEG_DIR}/include
    #linux
    /opt/libjpeg-turbo/include
    #mac
    /usr/local/opt/jpeg-turbo/include
    )
  
  FIND_LIBRARY(TURBOJPEG_LIBRARY
    NAMES turbojpeg
    PATHS 
    $ENV{LIBJPEG_TURBO_DIR}/lib
    $ENV{TURBOJPEG_DIR}/lib
    #linux
    /opt/libjpeg-turbo/lib
    #mac
    /usr/local/opt/jpeg-turbo/lib
    )

  INCLUDE_DIRECTORIES(${TURBOJPEG_INCLUDE_DIR})

ELSE()
  SET(TURBOJPEG_LIBRARY "")
ENDIF()

ADD_LIBRARY(ospray_dw_common
  WallConfig.cpp
  CompressedTile.cpp
  MPI.cpp
  Socket.cpp
  )

TARGET_LINK_LIBRARIES(ospray_dw_common
  ${MPI_CXX_LIBRARIES}
  ${TURBOJPEG_LIBRARY}
  ospray
  )


