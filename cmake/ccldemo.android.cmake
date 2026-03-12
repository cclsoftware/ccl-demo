include_guard (DIRECTORY)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/platform/android/androidmain.cpp
)

source_group ("source\\core" FILES ${corelib_android_jnionload_sources})

list (APPEND ccldemo_ccl_sources
	${corelib_android_jnionload_sources}
)

ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/vertexshader.vert PATH shaders)
ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/pixelshader.frag PATH shaders)

ccl_add_deployment_project (ccldemo "dev.ccl.ccldemo")
