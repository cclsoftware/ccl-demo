include_guard (DIRECTORY)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/platform/cocoa/cocoamain.mm
)

list (APPEND ccldemo_bundle_resources
	${CMAKE_CURRENT_LIST_DIR}/../packaging/mac/resource/ccldemo.xcassets
	${CCL_DIR}/packaging/mac/resource/MainMenu.xib
)

list (APPEND ccldemo_resources
	${ccldemo_bundle_resources}
)

ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/vertexshader.metal PATH Resources/shaders)
ccl_add_shader_resource (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../resource/shaders/pixelshader.metal PATH Resources/shaders)

list (APPEND ccldemo_sources
	${ccldemo_resources}
)
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

if (${CCLDEMO_CCLSPY_SUPPORT})
	add_plugin_to_mac_debug_bundle(ccldemo cclspy)
endif ()

# set packaging properties
set_target_properties (ccldemo PROPERTIES
	RESOURCE "${ccldemo_bundle_resources}"
	MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/../packaging/mac/Info.plist"
	XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"

	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
	XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${entitlements_output_file}"
)

if (${VENDOR_USE_PUBLISHER_CERTIFICATE})
	set_target_properties (ccldemo PROPERTIES
		XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual"
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${SIGNING_CERTIFICATE_MAC}"
	)
endif ()
