
ADD_EXECUTABLE(ospDwTest
  testMain.cpp
  )
TARGET_LINK_LIBRARIES(ospDwTest
  ospray_dw_common
  )