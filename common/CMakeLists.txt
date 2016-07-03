
ADD_LIBRARY(ospray_dw_common
  WallConfig.cpp
  )

TARGET_LINK_LIBRARIES(ospray_dw_common
  ${MPI_CXX_LIBRARIES}
  ospray
  )
  

