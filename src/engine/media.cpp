#include "media.hpp"

namespace engine::media
{
	std::initializer_list<std::pair<AVChannelLayout, ChannelSet>> channel_layout_table
	{
		{ AV_CHANNEL_LAYOUT_MONO, ChannelSet{ Channel::MONO_CHANNEL }},
		{ AV_CHANNEL_LAYOUT_STEREO, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT }},
		{ AV_CHANNEL_LAYOUT_2POINT1, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::LFE }},
		{ AV_CHANNEL_LAYOUT_3POINT1, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::FRONT_CENTER, Channel::LFE }},
		{ AV_CHANNEL_LAYOUT_4POINT0, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::REAR_LEFT, Channel::REAR_RIGHT }},
		{ AV_CHANNEL_LAYOUT_4POINT1, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::REAR_LEFT, Channel::REAR_RIGHT, Channel::LFE }},
		{ AV_CHANNEL_LAYOUT_5POINT0, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::REAR_LEFT, Channel::REAR_RIGHT, Channel::FRONT_CENTER }},
		{ AV_CHANNEL_LAYOUT_5POINT1, ChannelSet{ Channel::FRONT_LEFT, Channel::FRONT_RIGHT, Channel::REAR_LEFT, Channel::REAR_RIGHT, Channel::FRONT_CENTER, Channel::LFE }}
	};

	AVChannelLayout channelsToLayout(ChannelSet channels)
	{
		for (auto& option : channel_layout_table)
			if (option.second == channels)
				return option.first;

		throw std::exception("Layout is not supported.");
	}

	ChannelSet layoutToChannels(AVChannelLayout layout)
	{
		for (auto& option : channel_layout_table)
			if (option.first.nb_channels == layout.nb_channels && option.first.order == layout.order)
				return option.second;

		throw std::exception("Layout is not supported.");
	}

	AudioSource::AudioSource(SampleCount sample_rate, ChannelSet channels) : sample_rate(sample_rate), channels(channels) {}

	ChannelSamples BufferedAudioSource::pullSamples(SampleCount amount)
	{
		if (amount > sample_storage.begin()->second.size())
			fillSamples(amount - sample_storage.begin()->second.size());

		ChannelSamples buffer;

		for (auto& pair : sample_storage)
		{
			auto start = pair.second.begin();
			auto stop = start + std::min(amount, pair.second.size());

			buffer[pair.first] = SampleVector(start, stop);
			pair.second.erase(start, stop);
		}

		return buffer;
	}

	BufferedAudioSource::BufferedAudioSource(SampleCount sample_rate, ChannelSet channels) : AudioSource(sample_rate, channels)
	{
		for (auto& channel : channels)
			sample_storage[channel] = SampleVector();
	}

	void BufferedAudioSource::pushSamples(ChannelSamples samples)
	{
		for (auto& pair : sample_storage)
			pair.second.insert(pair.second.begin(), samples[pair.first].begin(), samples[pair.first].end());
	}

	void BufferedAudioSource::fillSamples(SampleCount amount) {}

	AudioBuffer::AudioBuffer(SampleCount sample_rate, ChannelSet channels) : BufferedAudioSource(sample_rate, channels) {}

	AudioBuffer::AudioBuffer(SampleCount sample_rate, ChannelSamples samples) : BufferedAudioSource(sample_rate, ChannelSet(std::views::keys(samples).begin(), std::views::keys(samples).end()))
	{
		BufferedAudioSource::pushSamples(samples);
	}

	void AudioBuffer::pushSamples(ChannelSamples samples)
	{
		BufferedAudioSource::pushSamples(samples);
	}

	AudioMixer::AudioMixer(SampleCount sample_rate, ChannelSet channels) : AudioSource(sample_rate, channels) {}

	bool AudioMixer::insertSource(SharedAudioSource source)
	{
		if (sources.contains(source))
			return false;

		sources.insert(source);
		return true;
	}

	bool AudioMixer::removeSource(SharedAudioSource source)
	{
		if (!sources.contains(source))
			return false;

		sources.erase(source);
		return true;
	}

	ChannelSamples AudioMixer::pullSamples(SampleCount amount)
	{
		ChannelSamples buffer;

		for (auto& channel : channels)
			buffer[channel] = SampleVector(amount);

		for (auto& source : sources)
		{
			auto samples = source->pullSamples(amount);

			for (auto& channel : channels)
				for (SampleCount i = 0; i < samples[channel].size(); i++)
					buffer[channel][i] += samples[channel][i];
		}

		return buffer;
	}

	bool AudioDrain::insertSource(SharedAudioSource source)
	{
		return AudioMixer::insertSource(source);
	}

	bool AudioDrain::removeSource(SharedAudioSource source)
	{
		return AudioMixer::removeSource(source);
	}

	AudioDrain::AudioDrain(SampleCount sample_rate, ChannelSet channels) : AudioMixer(sample_rate, channels), sample_rate(sample_rate), channels(channels) {}

	AudioResampler::AudioResampler(SampleCount sample_rate, SharedAudioSource source) : BufferedAudioSource(sample_rate, source->channels), channel_layout(channelsToLayout(channels))
	{
		this->source = source;

		resampler_context = swr_alloc();

		av_opt_set_int(resampler_context, "in_sample_rate", source->sample_rate, 0);
		av_opt_set_int(resampler_context, "out_sample_rate", sample_rate, 0);

		av_opt_set_chlayout(resampler_context, "in_chlayout", &channel_layout, 0);
		av_opt_set_chlayout(resampler_context, "out_chlayout", &channel_layout, 0);

		av_opt_set_sample_fmt(resampler_context, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
		av_opt_set_sample_fmt(resampler_context, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

		swr_init(resampler_context);
	}

	AudioResampler::~AudioResampler()
	{
		swr_free(&resampler_context);
	}

	//TODO: fix slow code
	void AudioResampler::fillSamples(SampleCount amount)
	{
		uint8_t** src_data = 0;
		int32_t src_linesize = 0;
		
		uint8_t** dst_data = 0;
		int32_t dst_linesize = 0;

		auto src_nb_channels = channel_layout.nb_channels;
		auto dst_nb_channels = channel_layout.nb_channels;

		auto src_rate = source->sample_rate;
		auto dst_rate = sample_rate;

		SampleCount src_nb_samples = av_rescale_rnd(amount, src_rate, dst_rate, AV_ROUND_UP);

		ChannelSamples original_samples = source->pullSamples(src_nb_samples);
		src_nb_samples = original_samples.begin()->second.size();

		SampleCount dst_nb_samples = av_rescale_rnd(src_nb_samples + swr_get_delay(resampler_context, src_rate), dst_rate, src_rate, AV_ROUND_UP);

		av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels, src_nb_samples, AV_SAMPLE_FMT_FLT, 0);
		av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, AV_SAMPLE_FMT_FLT, 0);

		SampleVector src_buff;
		for (int i = 0; i < src_nb_samples; i++)
			for (auto& channel : channels)
				src_buff.push_back(original_samples[channel][i]);

		std::copy(src_buff.begin(), src_buff.end(), (float_t*)src_data[0]);

		int ret = swr_convert(resampler_context, dst_data, dst_nb_samples, (const uint8_t**)src_data, src_nb_samples);
		int32_t dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, ret, AV_SAMPLE_FMT_FLT, 1);

		std::vector<Sample> dst_buff(dst_bufsize / sizeof(Sample));
		std::copy_n((float_t*)dst_data[0], dst_bufsize / sizeof(Sample), dst_buff.begin());

		ChannelSamples buffer;
		
		for (auto& channel : channels)
			buffer[channel] = SampleVector(dst_nb_samples);

		auto iter = dst_buff.begin();
		for (int i = 0; i < dst_bufsize / sizeof(Sample) / dst_nb_channels; i++)
			for (auto& channel : channels)
			{
				buffer[channel].push_back(*iter);
				std::advance(iter, 1);
			}

		if (src_data)
			av_freep(&src_data[0]);

		if (dst_data)
			av_freep(&dst_data[0]);

		av_freep(&src_data);
		av_freep(&dst_data);

		pushSamples(buffer);
	}

	SDLAudioDrain::SDLAudioDrain(SampleCount sample_rate, ChannelSet channels) : AudioDrain(sample_rate, channels)
	{
		if (channels.size() < 1 || channels.size() > 2)
			throw std::exception("Not supported channel amount.");

		SDL_AudioSpec des = { 0 };

		des.channels = channels.size();
		des.format = AUDIO_F32;
		des.freq = sample_rate;
		des.userdata = this;

		des.callback = [](void* userdata, uint8_t* stream, int len)
		{
			std::fill_n(stream, len, 0);

			auto drain = static_cast<media::SDLAudioDrain*>(userdata);
			auto samples = drain->pullSamples(len / sizeof(Sample) / 1); //drain->channels.size()

			if (drain->channels.size() == 1)
			{
				auto& buffer = samples.begin()->second;
				std::copy(buffer.begin(), buffer.end(), (Sample*)stream);
			}
			else
			{
				auto& first_buffer = samples.begin()->second;
				auto& second_buffer = samples.rbegin()->second;

				for (SampleCount i = 0; i < first_buffer.size(); i++)
				{
					std::copy(first_buffer.begin() + i, first_buffer.begin() + i + 1, (Sample*)stream + i);
					std::copy(second_buffer.begin() + i, second_buffer.begin() + i + 1, (Sample*)stream + i);
				}
			}
		};

		SDL_AudioSpec obt = { 0 };
		
		audio_device = SDL_OpenAudioDevice(NULL, NULL, &des, &obt, 0);
		if (!audio_device)
			throw std::exception("No audio device.");

		if (obt.freq != des.freq || obt.format != des.format || obt.channels != des.channels)
		{
			SDL_CloseAudioDevice(audio_device);
			throw std::exception("Wrong device.");
		}

		SDL_PauseAudioDevice(audio_device, false);
	}

	SDLAudioDrain::~SDLAudioDrain()
	{
		SDL_PauseAudioDevice(audio_device, true);
		SDL_CloseAudioDevice(audio_device);
	}

	ChannelSamples engine::media::SDLAudioDrain::pullSamples(SampleCount amount)
	{
		return AudioMixer::pullSamples(amount);
	}

	ToneGenerator::ToneGenerator(uint32_t frequency, Waveform wavetype, uint32_t sample_rate, double_t phase) : frequency(frequency), wavetype(wavetype), phase(phase), AudioSource(sample_rate == 0 ? frequency * 2 : sample_rate, { Channel::MONO_CHANNEL }) {}

	ChannelSamples ToneGenerator::pullSamples(SampleCount amount)
	{
		SampleVector buffer;

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

		ChannelSamples buffer_buffer;
		buffer_buffer[Channel::MONO_CHANNEL] = buffer;

		return buffer_buffer;
	}

	AudioBuffer AudioLoader::loadAudio(std::string url)
	{
		std::map<Channel, SampleVector> samples;

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
				SampleVector buf;

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
}