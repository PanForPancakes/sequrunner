#include "media.hpp"

namespace engine::media
{
	std::initializer_list<std::pair<AVChannelLayout, Channels>> channel_layout_table
	{
		{ AV_CHANNEL_LAYOUT_MONO, Channels{ Channel::MONO_SOUND }},
		{ AV_CHANNEL_LAYOUT_STEREO, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT }},
		{ AV_CHANNEL_LAYOUT_2POINT1, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::LFE }},
		{ AV_CHANNEL_LAYOUT_3POINT1, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::FRONT_CENTER, Channel::LFE }},
		{ AV_CHANNEL_LAYOUT_4POINT0, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::SURROUND_LEFT, Channel::SURROUND_RIGHT }},
		{ AV_CHANNEL_LAYOUT_4POINT1, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::SURROUND_LEFT, Channel::SURROUND_RIGHT, Channel::LFE }},
		{ AV_CHANNEL_LAYOUT_5POINT0, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::SURROUND_LEFT, Channel::SURROUND_RIGHT, Channel::FRONT_CENTER }},
		{ AV_CHANNEL_LAYOUT_5POINT1, Channels{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::SURROUND_LEFT, Channel::SURROUND_RIGHT, Channel::FRONT_CENTER, Channel::LFE }}
	};

	AVChannelLayout channelsToLayout(Channels channels)
	{
		for (auto& option : channel_layout_table)
			if (option.second == channels)
				return option.first;

		throw std::exception("Layout is not supported.");
	}

	Channels layoutToChannels(AVChannelLayout layout)
	{
		for (auto& option : channel_layout_table)
			if (option.first.nb_channels == layout.nb_channels && option.first.order == layout.order)
				return option.second;

		throw std::exception("Layout is not supported.");
	}
}