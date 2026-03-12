include_guard (DIRECTORY)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/platform/cocoa/cocoamain.mm
)

list (APPEND ccldemo_bundle_resources
	${CMAKE_CURRENT_LIST_DIR}/../packaging/ios/resource/launchscreen.storyboard
	${CCL_DIR}/packaging/ios/resource/cclapp_ios.xcassets
)

list (APPEND ccldemo_resources
	${ccldemo_bundle_resources}
)

list (APPEND ccldemo_sources
	${ccldemo_resources}
)

ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/vertexshader.metal PATH shaders)
ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/pixelshader.metal PATH shaders)

source_group ("Resources" FILES ${ccldemo_resources})

set (ccldemo_FRAMEWORKS
	ccltext
	cclsystem
	cclgui
	cclnet
	cclsecurity
)

set (ccldemo_PLUGINS
	modelimporter3d
)

set_target_properties (ccldemo PROPERTIES	
	XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
	RESOURCE "${ccldemo_bundle_resources}"
	MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/../packaging/ios/Info.plist"
	XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "cclapp"
	XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_LIST_DIR}/../packaging/ios/ccldemo.entitlements"
)
