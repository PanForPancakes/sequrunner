file (READ "imgui-cmake/CMakeLists.txt" FILE_CONTENT)
string (REPLACE "elseif (EXISTS ../freetype)" "elseif (EXISTS \${CMAKE_CURRENT_SOURCE_DIR}/../freetype)" FILE_CONTENT "${FILE_CONTENT}")
file (WRITE "imgui/CMakeLists.txt" "${FILE_CONTENT}")
set (FILE_CONTENT "")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg")
	message (
		FATAL_ERROR
		"Please download and unpack prebuilt ffmpeg into external/ffmpeg directory, then regenerate project.\n"
		"For example you can get such prebuilt ffmpeg at https://github.com/BtbN/FFmpeg-Builds/releases/tag/latest"
	)
endif ()

add_custom_command (
	TARGET ALL
	PRE_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/FFmpeg/bin/ $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

set (IMGUI_ENABLE_STDLIB_SUPPORT ON)

add_subdirectory ("poco" EXCLUDE_FROM_ALL)
add_subdirectory ("SDL" EXCLUDE_FROM_ALL)
add_subdirectory ("imgui" EXCLUDE_FROM_ALL)