#pragma once

#include <algorithm>
#include <numbers>
#include <vector>
#include <memory>
#include <string>
#include <ranges>
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

#include <SDL.h>

#include "utility.hpp"

namespace engine::media
{
	using namespace engine::utility;

	enum class Channel
	{
		MONO_CHANNEL = 0,
		FRONT_CENTER = 0,
		FRONT_LEFT,
		FRONT_RIGHT,
		REAR_LEFT,
		REAR_RIGHT,
		LFE
	};

	typedef Set<Channel> ChannelSet;
	typedef Map<Channel, SampleVector> ChannelSamples;

	extern std::initializer_list<std::pair<AVChannelLayout, ChannelSet>> channel_layout_table;

	AVChannelLayout channelsToLayout(ChannelSet channels);
	ChannelSet layoutToChannels(AVChannelLayout layout);

	class AudioSource
	{
	public:
		const SampleCount sample_rate;
		const ChannelSet channels;

		virtual ChannelSamples pullSamples(SampleCount amount) = 0;

	protected:
		AudioSource(SampleCount sample_rate, ChannelSet channels);
	};

	typedef Shared<AudioSource> SharedAudioSource;

	class BufferedAudioSource : public AudioSource
	{
		ChannelSamples sample_storage;

	public:
		ChannelSamples pullSamples(SampleCount amount) override;

	protected:
		BufferedAudioSource(SampleCount sample_rate, ChannelSet channels);

		void pushSamples(ChannelSamples samples);
	};

	class AudioBuffer : public BufferedAudioSource
	{
	public:
		AudioBuffer(SampleCount sample_rate, ChannelSet channels);
		AudioBuffer(SampleCount sample_rate, ChannelSamples samples);

		void pushSamples(ChannelSamples samples);
	};

	class AudioMixer : public AudioSource
	{
		Set<SharedAudioSource> sources;

	public:
		AudioMixer(SampleCount sample_rate, ChannelSet channels);

		bool insertSource(SharedAudioSource source);
		bool removeSource(SharedAudioSource source);

		ChannelSamples pullSamples(SampleCount amount) override;
	};

	class AudioDrain : protected AudioMixer
	{
	public:
		const SampleCount sample_rate;
		const ChannelSet channels;

		bool insertSource(SharedAudioSource source);
		bool removeSource(SharedAudioSource source);

	protected:
		AudioDrain(SampleCount sample_rate, ChannelSet channels);
	};

	class AudioResampler : public AudioSource
	{
	public:
		AudioResampler(SampleCount sample_rate, SharedAudioSource source);

		ChannelSamples pullSamples(SampleCount amount) override;
	};

	class SDLAudioDrain : public AudioDrain
	{
	public:
		SDLAudioDrain(SampleCount sample_rate, ChannelSet channels);
		~SDLAudioDrain();

		ChannelSamples pullSamples(SampleCount amount);
	};

	class ToneGenerator : public AudioSource
	{
		double_t phase = 0;

	public:
		enum class Waveform { SINE, SQUARE, TRIANGLE, SAWTOOTH } const wavetype;
		const uint32_t frequency;

		ToneGenerator(uint32_t frequency, Waveform wavetype, uint32_t sample_rate = 0, double_t phase = 0);

		ChannelSamples pullSamples(SampleCount amount) override;
	};

	/*class AudioResampler : public AudioSource, public AbstractAudioSink
	{
		SwrContext* resampler_context;

		uint32_t source_sample_rate = 0;
		double_t sample_rate_multiplier = 0;

		const AVChannelLayout channel_layout;

		void reconfigureResampler(SwrContext* context);

	public:
		AudioResampler(uint32_t out_sample_rate, Channels channels);
		~AudioResampler();

		void insertSource(std::shared_ptr<AudioSource> source);
		SampleVector pullSamples(uint32_t amount, Channel channel);
	};*/

	/*struct AudioLoader
	{
		static AudioBuffer loadAudio(std::string url);
	};*/
}