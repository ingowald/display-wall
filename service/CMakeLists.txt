

#IF (OSPRAY_MPI)

# if mpi mode is enabled, we have to configure the right mpi
# compiler etc.
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/glut.cmake)

ADD_EXECUTABLE(ospDisplayWald
  main.cpp
  GlutWindow.cpp
  Dispatcher.cpp
  )

TARGET_LINK_LIBRARIES(ospDisplayWald
  ospray_dw_common
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  )
  
