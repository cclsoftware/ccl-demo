include_guard (DIRECTORY)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/platform/win/winmain.cpp
)

list (APPEND ccldemo_icons
	1	${CMAKE_CURRENT_LIST_DIR}/../packaging/win/resource/ccldemo.ico
)

ccl_add_vertexshader (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/vertexshader.hlsl PATH shaders)
ccl_add_pixelshader (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/pixelshader.hlsl PATH shaders)

ccl_embed_manifest (ccldemo "${CCL_DIR}/packaging/win/application.manifest")

ccl_nsis_package (ccldemo "${CMAKE_CURRENT_LIST_DIR}/../packaging/win/nsis/demoapp.nsi" "/DCCL_APPLICATIONS_DIR=${CMAKE_CURRENT_LIST_DIR}/../..")
