#pragma once

#include <algorithm>
#include <numbers>
#include <vector>
#include <memory>
#include <string>
#include <ranges>
#include <queue>
#include <deque>
#include <cmath>
#include <map>
#include <set>

extern "C"
{
	#include <libavdevice/avdevice.h>
	#include <libavfilter/avfilter.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}

#include <SDL.h>

#include "utility.hpp"

namespace engine::media
{
	using namespace engine::utility;


}