## Copyright (c) 2016-2017 Ingo Wald
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

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

PROJECT(DISPLAY_WALD)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(OSPRAY_DISPLAY_WALD_VERSION_MAJOR 0)
SET(OSPRAY_DISPLAY_WALD_VERSION_MINOR 1)
SET(OSPRAY_DISPLAY_WALD_VERSION_PATCH 0)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

IF (THIS_IS_OSPRAY)
  # ------------------------------------------------------------------
  # configure this as a ospray module ...
  # ------------------------------------------------------------------
  OPTION(OSPRAY_MODULE_DISPLAY_WALD "DisplayWald module" ON)

  SET(OSPRAY_DIR ${PROJECT_SOURCE_DIR})
ELSE ()
  # ------------------------------------------------------------------
  # build as stand-alone library
  # ------------------------------------------------------------------

  # when building as a stand-alone library we still need ospcommon and
  # components
  find_path(OSPRAY_DIR
    NAMES ospray/version.h.in
    PATHS /home/wald/Projects/ospray $ENV{OSPRAY_DIR}
    )
  IF (NOT OSPRAY_DIR)
    MESSAGE("OSPRAY DIR NOT FOUND")
  ENDIF()
  SET(OSPRAY_DIR /home/wald/Projects/ospray)
ENDIF()

SET(COMPONENTS_DIR ${OSPRAY_DIR}/components)
SET(OSPCOMMON_DIR ${COMPONENTS_DIR}/ospcommon)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${OSPCOMMON_DIR}/cmake)

INCLUDE(glut)
INCLUDE(mpi)

INCLUDE(macros)
OSPRAY_CONFIGURE_COMPILER()
CONFIGURE_MPI()

INCLUDE_DIRECTORIES(${COMPONENTS_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(${COMPONENTS_DIR} ${CMAKE_BINARY_DIR}/builddir-components)

ADD_SUBDIRECTORY(common EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(service)
ADD_SUBDIRECTORY(client)
  
# the ospray pixelop one can use to get a ospray frame buffer to the
# display wall
IF (OSPRAY_MODULE_DISPLAY_WALD)
  
  INCLUDE_DIRECTORIES(${OSPRAY_DIR}/ospray/include)
  SET(OSPRAY_DISPLAY_WALD_VERSION
    ${OSPRAY_DISPLAY_WALD_VERSION_MAJOR}.${OSPRAY_DISPLAY_WALD_VERSION_MINOR}.${OSPRAY_DISPLAY_WALD_VERSION_PATCH})
  
  ADD_SUBDIRECTORY(ospray)

ELSE()

  ADD_SUBDIRECTORY(apps)

ENDIF()

