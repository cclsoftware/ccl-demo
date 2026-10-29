include_guard (DIRECTORY)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/platform/wasm/wasmmain.cpp
)

ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/vertexshader.vert PATH shaders)
ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/pixelshader.frag PATH shaders)

install (TARGETS ccldemo RUNTIME DESTINATION ${VENDOR_APPLICATION_RUNTIME_DIRECTORY})

set (CPACK_PACKAGE_VERSION_MAJOR ${ccldemo_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${ccldemo_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${ccldemo_VERSION_REVISION})
set (CPACK_PACKAGE_VERSION "${ccldemo_VERSION_MAJOR}.${ccldemo_VERSION_MINOR}.${ccldemo_VERSION_REVISION}.${ccldemo_VERSION_BUILD}")
include (CPack)
