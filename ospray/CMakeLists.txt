
OSPRAY_ADD_LIBRARY(ospray_module_displayWald SHARED
  OSPRayModule.cpp
  )

TARGET_LINK_LIBRARIES(ospray_module_displayWald
  ospray_displayWald_client
	ospray
  )
