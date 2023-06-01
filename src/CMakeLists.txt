add_executable (
	${PROJECT_NAME}
	"main.cpp"
	"ui.cpp"

	"engine/playback.cpp"
	"engine/media.cpp"
	"engine/utility.cpp"

	"${CMAKE_SOURCE_DIR}/external/imgui/misc/cpp/imgui_stdlib.cpp"
	"${CMAKE_SOURCE_DIR}/external/imgui-knobs/imgui-knobs.cpp"
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
	C_STANDARD 20
	CXX_STANDARD 20
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
	"${CMAKE_SOURCE_DIR}/external/ffmpeg/include"
	"${CMAKE_SOURCE_DIR}/external/imgui-knobs"
	"${CMAKE_SOURCE_DIR}/external/nlohmann-json"
)

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
	"assets"
	"opengl32"
	"SDL2::SDL2"
	"SDL2::SDL2main"
	"imgui"
	"imgui-sdl2"
	"imgui-sdlrenderer"
	"Poco::Foundation"
	"Poco::Zip"
	"avformat"
	"avcodec"
	"avdevice"
	"avfilter"
	"avutil"
	"swscale"
	"swresample"
	"postproc"
)

target_link_directories (
	${PROJECT_NAME}
	PRIVATE
	"${CMAKE_SOURCE_DIR}/external/ffmpeg/lib"
)