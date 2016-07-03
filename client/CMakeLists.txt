
ADD_EXECUTABLE(ospDwTest
  testMain.cpp
	Client.cpp
  )
TARGET_LINK_LIBRARIES(ospDwTest
  ospray_dw_common
  )