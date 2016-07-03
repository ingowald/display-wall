## ======================================================================== ##
## Copyright 2009-2015 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

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
