#pragma once

#include <algorithm>
#include <numbers>
#include <vector>
#include <memory>
#include <string>
#include <queue>
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

#include "utility.hpp"

namespace engine::media
{
	using namespace engine::utility;

	enum class Channel
	{
		MONO_SOUND = 0,
		FRONT_CENTER = 0,
		FRONT_LEFT,
		FRONT_RIGHT,
		LFE,
		SURROUND_LEFT,
		SURROUND_RIGHT
	};

	typedef std::set<Channel> Channels;

	extern std::initializer_list<std::pair<AVChannelLayout, Channels>> channel_layout_table;

	AVChannelLayout channelsToLayout(Channels channels);
	Channels layoutToChannels(AVChannelLayout layout);

	struct AbstractAudioSource
	{
		const uint32_t sample_rate;
		const Channels channels;

		AbstractAudioSource(uint32_t sample_rate, Channels channels) : sample_rate(sample_rate), channels(channels) {}

		virtual AudioSampleVector pullSamples(uint32_t amount, Channel channel) = 0;
	};

	struct AbstractAudioSink
	{
		std::shared_ptr<AbstractAudioSource> input;

		virtual void insertSource(std::shared_ptr<AbstractAudioSource> source);
	};

	enum class Waveform
	{
		SINE,
		SQUARE,
		TRIANGLE,
		SAWTOOTH
	};

	class ToneGenerator : public AbstractAudioSource
	{
		double_t phase = 0;

	public:
		Waveform wavetype;

		const uint32_t frequency;

		ToneGenerator(uint32_t frequency, Waveform wavetype, uint32_t sample_rate = 0, double_t phase = 0);

		AudioSampleVector pullSamples(uint32_t amount, Channel channel) override;
	};

	class AudioBuffer : public AbstractAudioSource
	{
		std::map<Channel, AudioSampleVector> samples;

		static Channels getChannels(const std::map<Channel, AudioSampleVector>& samples);

	public:
		AudioBuffer(uint32_t sample_rate, std::map<Channel, AudioSampleVector> samples);

		AudioSampleVector pullSamples(uint32_t amount, Channel channel) override;
	};

	class AudioResampler : public AbstractAudioSource, public AbstractAudioSink
	{
		SwrContext* resampler_context;

		uint32_t source_sample_rate = 0;
		double_t sample_rate_multiplier = 0;

		const AVChannelLayout channel_layout;

		void reconfigureResampler(SwrContext* context);

	public:
		AudioResampler(uint32_t out_sample_rate, Channels channels);
		~AudioResampler();

		void insertSource(std::shared_ptr<AbstractAudioSource> source);
		AudioSampleVector pullSamples(uint32_t amount, Channel channel);
	};

	struct AudioLoader
	{
		static AudioBuffer loadAudio(std::string url);
	};
}