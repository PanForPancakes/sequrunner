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

		virtual void insertSource(std::shared_ptr<AbstractAudioSource> source)
		{
			input = source;
		}
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

		ToneGenerator(uint32_t frequency, Waveform wavetype, uint32_t sample_rate = 0, double_t phase = 0) : frequency(frequency), wavetype(wavetype), phase(phase), AbstractAudioSource(sample_rate == 0 ? frequency * 2 : sample_rate, { Channel::MONO_SOUND }) {}

		AudioSampleVector pullSamples(uint32_t amount, Channel channel) override
		{
			AudioSampleVector buffer;

			if (wavetype == Waveform::SINE)
			{
				for (uint32_t i = 0; i < amount; i++)
				{
					buffer.push_back(std::sinf(frequency * phase * (float)std::numbers::pi));

					phase += 1.0f / (double_t)sample_rate;
					phase = phase > 1.0f ? phase - 1.0f : phase;
				}
			}
			else if (wavetype == Waveform::SQUARE)
			{
				for (uint32_t i = 0; i < amount; i++)
				{
					buffer.push_back(std::sinf(frequency * phase * (float)std::numbers::pi) > 0 ? 1.0f : -1.0f);

					phase += 1.0f / (double_t)sample_rate;
					phase = phase > 1.0f ? phase - 1.0f : phase;
				}
			}
			else if (wavetype == Waveform::TRIANGLE)
			{
				for (uint32_t i = 0; i < amount; i++)
				{
					buffer.push_back((2 / (float)std::numbers::pi) * std::asinf(std::sinf(frequency * phase * (float)std::numbers::pi)));

					phase += 1.0f / (double_t)sample_rate;
					phase = phase > 1.0f ? phase - 1.0f : phase;
				}
			}
			else if (wavetype == Waveform::SAWTOOTH)
			{
				for (uint32_t i = 0; i < amount; i++)
				{
					buffer.push_back(std::fmodf(frequency * phase / 2, 1) * 2.0f - 1.0f);

					phase += 1.0f / (double_t)sample_rate;
					phase = phase > 1.0f ? phase - 1.0f : phase;
				}
			}

			return buffer;
		}
	};

	class AudioBuffer : public AbstractAudioSource
	{
		std::map<Channel, AudioSampleVector> samples;

		static Channels getChannels(const std::map<Channel, AudioSampleVector>& samples)
		{
			Channels buffer;

			for (auto& pair : samples)
				buffer.insert(pair.first);

			return buffer;
		}

	public:
		AudioBuffer(uint32_t sample_rate, std::map<Channel, AudioSampleVector> samples) : AbstractAudioSource(sample_rate, getChannels(samples)), samples(samples) {}

		AudioSampleVector pullSamples(uint32_t amount, Channel channel) override
		{
			AudioSampleVector buffer;

			if (samples[channel].size() >= amount)
			{
				buffer = AudioSampleVector(samples[channel].begin(), samples[channel].begin() + amount);
				samples[channel].erase(samples[channel].begin(), samples[channel].begin() + amount);
			}
			else
			{
				buffer = AudioSampleVector(samples[channel].begin(), samples[channel].end());
				samples[channel].clear();
			}

			return buffer;
		}
	};

	class AudioResampler : public AbstractAudioSource, public AbstractAudioSink
	{
		SwrContext* resampler_context;

		uint32_t source_sample_rate = 0;
		double_t sample_rate_multiplier = 0;

		const AVChannelLayout channel_layout;

		void reconfigureResampler(SwrContext* context)
		{
			if (swr_is_initialized(context))
				swr_close(context);

			av_opt_set_int(context, "in_sample_rate", source_sample_rate, 0);
			av_opt_set_int(context, "out_sample_rate", sample_rate, 0);

			av_opt_set_chlayout(context, "in_chlayout", &channel_layout, 0);
			av_opt_set_chlayout(context, "out_chlayout", &channel_layout, 0);

			av_opt_set_sample_fmt(context, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
			av_opt_set_sample_fmt(context, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

			swr_init(context);
		}

	public:
		AudioResampler(uint32_t out_sample_rate, Channels channels) : AbstractAudioSource(out_sample_rate, channels), channel_layout(channelsToLayout(channels))
		{
			resampler_context = swr_alloc();
		}

		~AudioResampler()
		{
			swr_free(&resampler_context);
		}

		void insertSource(std::shared_ptr<AbstractAudioSource> source) override
		{
			AbstractAudioSink::insertSource(source);

			source_sample_rate = source->sample_rate;
			sample_rate_multiplier = sample_rate / source->sample_rate;

			reconfigureResampler(resampler_context);
		}

		AudioSampleVector pullSamples(uint32_t amount, Channel channel) override
		{
			AudioSampleVector buffer;

			if (!input)
				throw std::exception("Used pullSamples without input set.");

			AudioSampleVector original_samples = input->pullSamples(std::ceil(sample_rate_multiplier * amount), channel);

			//uint8_t** source_buffer = 0;
			//int32_t source_linesize = 0;
			//
			//uint8_t** destination_buffer = 0;
			//int32_t destination_linesize = 0;
			//
			//av_samples_alloc_array_and_samples(&source_buffer, &source_linesize, , dst_nb_samples, dst_sample_fmt, 0);
			//av_samples_alloc_array_and_samples(&destination_buffer, &destination_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 0);
			//
			//av_freep(&dst_data[0]);
			//av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 1);
			//
			//ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t**)src_data, src_nb_samples);
			//if (ret < 0) {
			//	fprintf(stderr, "Error while converting\n");
			//	goto end;
			//}
			//dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
			//	ret, dst_sample_fmt, 1);
			//if (dst_bufsize < 0) {
			//	fprintf(stderr, "Could not get sample buffer size\n");
			//	goto end;
			//}
			//printf("t:%f in:%d out:%d\n", t, src_nb_samples, ret);
			//fwrite(dst_data[0], 1, dst_bufsize, dst_file);
			//
			//swr_convert(resampler_context, )

			return buffer;
		}
	};

	struct AudioLoader
	{
		static AudioBuffer loadAudio(std::string url)
		{
			std::map<Channel, AudioSampleVector> samples;

			AVFormatContext* format_context = avformat_alloc_context();

			avformat_open_input(&format_context, url.c_str(), 0, 0);
			avformat_find_stream_info(format_context, 0);

			const AVCodec* decoder;
			int stream_index = av_find_best_stream(format_context, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);

			AVCodecContext* decoder_context = avcodec_alloc_context3(decoder);
			avcodec_parameters_to_context(decoder_context, format_context->streams[stream_index]->codecpar);

			avcodec_open2(decoder_context, decoder, 0);

			AVPacket* buffer_packet = av_packet_alloc();
			AVFrame* buffer_frame = av_frame_alloc();

			AVFrame* frame = av_frame_alloc();

			SwrContext* resampler_context = swr_alloc();

			bool resampler_prepared = false;
			while (!av_read_frame(format_context, buffer_packet))
			{
				if (buffer_packet->stream_index != stream_index)
					continue;

				avcodec_send_packet(decoder_context, buffer_packet);
				avcodec_receive_frame(decoder_context, buffer_frame);

				if (!resampler_prepared)
				{
					frame->sample_rate = buffer_frame->sample_rate;
					frame->ch_layout = buffer_frame->ch_layout;
					frame->format = AV_SAMPLE_FMT_FLTP;

					swr_config_frame(resampler_context, frame, buffer_frame);
					swr_init(resampler_context);

					resampler_prepared = true;
				}

				// yeah, it doesnt work, i have no idea why
				int ret = swr_convert_frame(resampler_context, frame, buffer_frame);
				auto frame = buffer_frame;
				if (ret < 0)
					throw std::exception("This is bug I stopped researching since it was taking too long to understand why it just can't do it. Use decoders that return floating point samples as a workaround.");

				for (int channel = 0; channel < decoder_context->ch_layout.nb_channels; channel++)
				{
					AudioSampleVector buf;

					for (int i = 0; i < frame->nb_samples; i++)
						buf.push_back(*((float_t*)(frame->data[channel]) + i));

					auto channel_channel = (Channel)(decoder_context->ch_layout.nb_channels == 1 ? 0 : channel + 1);
					samples[channel_channel].insert(samples[channel_channel].end(), buf.begin(), buf.end());
				}
			}

			auto sample_rate = frame->sample_rate;

			av_frame_free(&frame);

			av_packet_free(&buffer_packet);
			av_frame_free(&buffer_frame);

			swr_free(&resampler_context);

			avcodec_free_context(&decoder_context);
			avformat_free_context(format_context);

			return AudioBuffer(sample_rate, samples);
		}
	};
}