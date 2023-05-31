cmake_minimum_required (VERSION 3.20)

set (BUILD_SHARED_LIBS OFF)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project (
	"sequrunner"
	VERSION 0.0.1
	DESCRIPTION "Cue based playback engine for media"
	HOMEPAGE_URL "https://github.com/PanForPancakes/sequrunner"
	LANGUAGES C CXX
)

add_subdirectory ("external")
add_subdirectory ("assets")

add_subdirectory ("src")