if (NOT (EXISTS "external/ffmpeg"))
	message (
		FATAL_ERROR
		"Please download and unpack prebuilt ffmpeg into external/ffmpeg directory, then regenerate project.\n"
		"For example you can get such prebuilt ffmpeg at https://github.com/BtbN/FFmpeg-Builds/releases/tag/latest"
	)
endif ()

add_custom_command (
	TARGET ${PROJECT_NAME}
	PRE_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/FFmpeg/bin/ $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

set (IMGUI_ENABLE_STDLIB_SUPPORT ON)

add_subdirectory ("poco" EXCLUDE_FROM_ALL)
add_subdirectory ("SDL" EXCLUDE_FROM_ALL)
add_subdirectory ("imgui" EXCLUDE_FROM_ALL)