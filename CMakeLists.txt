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

CONFIGURE_OSPRAY()

# mini-scene graph viewer for _geometry_ (intentionally simple)
OPTION(OSPRAY_MODULE_DISPLAY_WALD "DisplayWald (testing)" ON)

IF (OSPRAY_MODULE_DISPLAY_WALD)
  INCLUDE_DIRECTORIES(${EMBREE_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES_ISPC(${EMBREE_INCLUDE_DIRS})

  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ospray/include)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/apps)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/apps/qtViewer)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

  CONFIGURE_MPI()

  ADD_SUBDIRECTORY(common)
  ADD_SUBDIRECTORY(service)
  ADD_SUBDIRECTORY(client)

ENDIF()
