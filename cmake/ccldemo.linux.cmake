include_guard (DIRECTORY)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/platform/linux/linuxmain.cpp
)

ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/vertexshader.vert PATH shaders)
ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/pixelshader.frag PATH shaders)

install (TARGETS ccldemo RUNTIME DESTINATION ${VENDOR_APPLICATION_RUNTIME_DIRECTORY})

ccl_install_desktop_file (${CMAKE_CURRENT_LIST_DIR}/../packaging/linux/resource/ccldemo.desktop.in "${ccldemo_PACKAGE_ID}")
ccl_install_icon (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../packaging/linux/resource/ccldemo.svg)

include (${CMAKE_CURRENT_LIST_DIR}/../packaging/linux/deb/ccldemo.package.cmake)
