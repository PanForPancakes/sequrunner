include ("${CMAKE_SOURCE_DIR}/external/cmakerc/CMakeRC.cmake")

cmrc_add_resource_library (assets)

cmrc_add_resources (
	assets
	"textures/audiocue.bmp"
	"textures/videocue.bmp"
	"textures/memocue.bmp"
	"textures/error.bmp"
	"textures/paused.bmp"
	"textures/playhead.bmp"
	"textures/sequrunner.bmp"
	"textures/playing.bmp"
	"textures/armcue.bmp"
	"textures/cameracue.bmp"
	"textures/devampcue.bmp"
	"textures/disarmcue.bmp"
	"textures/fadecue.bmp"
	"textures/gotocue.bmp"
	"textures/groupcue.bmp"
	"textures/lightcue.bmp"
	"textures/loadcue.bmp"
	"textures/miccue.bmp"
	"textures/midicue.bmp"
	"textures/midifilecue.bmp"
	"textures/networkcue.bmp"
	"textures/pausecue.bmp"
	"textures/resetcue.bmp"
	"textures/startcue.bmp"
	"textures/stopcue.bmp"
	"textures/targetcue.bmp"
	"textures/timecodecue.bmp"
	"textures/waitcue.bmp"
	"textures/textcue.bmp"
)

cmrc_add_resources (
	assets
	"misc/Roboto-Regular.ttf"
)