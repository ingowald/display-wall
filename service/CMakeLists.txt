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

# if mpi mode is enabled, we have to configure the right mpi
# compiler etc.
#INCLUDE(${PROJECT_SOURCE_DIR}/cmake/glut.cmake)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/apps/exampleViewer/common/glfw/include)

ADD_EXECUTABLE(ospDisplayWald
  main.cpp
  #  GlutWindow.cpp
  glfwWindow.cpp
  Dispatcher.cpp
  processIncomingTiles.cpp
  Server.cpp
  )

ADD_EXECUTABLE(ospLaunchDisplayWald
  ../common/tinyxml/tinyxml.cpp
  ../common/tinyxml/tinystr.cpp
  ../common/tinyxml/tinyxmlerror.cpp
  ../common/tinyxml/tinyxmlparser.cpp
  launch.cpp
  )

TARGET_LINK_LIBRARIES(ospDisplayWald
  ospray_dw_common
#  ${OPENGL_LIBRARIES}
#  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  )
  
