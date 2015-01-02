#include <iostream>
#include <mutex>
#include <keyfinder/keyfinder.h>
#include <keyfinder/constants.h>

extern "C"
{
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
}

#include "key_notations.h"

/**
 * The "safe" AVPacket wrapper will handle memory managment of the packet,
 * ensuring that if an instance of this packet wrapper is destroyed the
 * containing packet is freed from memory.
 */
struct SafeAVPacket
{
    AVPacket inner_packet;

    SafeAVPacket()
    {
        av_init_packet(&inner_packet);

        inner_packet.data = nullptr;
        inner_packet.size = 0;
    }

    ~SafeAVPacket()
    {
        if (inner_packet.data)
        {
            av_free_packet(&inner_packet);
        }
    }

    void fill_packet(AVFormatContext* format_context)
    {
        if (inner_packet.data)
        {
            av_free_packet(&inner_packet);
        }

        if (av_read_frame(format_context, &inner_packet) < 0)
        {
            inner_packet.data = nullptr;
        }
    }
};

/**
 * Fill an instance of KeyFinder::AudioData with the audio data from a file.
 */
void fill_audio_data(const char* file_path, KeyFinder::AudioData &audio)
{
    // Initalize AV format/codec things once
    static std::once_flag init_flag;
    std::call_once(init_flag, []() { av_register_all(); });

    std::shared_ptr<AVFormatContext> format(avformat_alloc_context(), &avformat_free_context);
    auto format_ptr = format.get();

    // Open the file for decoding
    if (avformat_open_input(&format_ptr, file_path, nullptr, nullptr) < 0)
        throw std::runtime_error("Unable to load media file (probably invalid format)");

    // Deterine stream information
    if (avformat_find_stream_info(format_ptr, nullptr) < 0)
        throw std::runtime_error("Unable to get stream info");

    const AVStream* audio_stream = nullptr;

    // Get the audio stream from the context. We have to look through all of
    // the streams, we pick the first one that is recognized as an audio stream
    for (unsigned int i = 0; i < format->nb_streams; ++i)
    {
        auto stream = format->streams[i];

        if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_stream = stream;
            break;
        }
    }

    if (audio_stream == nullptr)
        throw std::runtime_error("File does not have any audio streams");

    // Get the codec context and codec handler
    const auto codec_context = audio_stream->codec;
    const auto codec = avcodec_find_decoder(codec_context->codec_id);

    if (codec == nullptr)
        throw std::runtime_error("Unsupported audio stream");

    // Open the codec
    if (avcodec_open2(codec_context, codec, nullptr) < 0)
        throw std::runtime_error("Unable to open the codec");

    // Setup the audio resample context in situations where we need to resample
    // the audio stream samples into 16bit PCM data
    std::shared_ptr<AVAudioResampleContext> resample_context(
            avresample_alloc_context(),
            [](AVAudioResampleContext* c) { avresample_free(&c); });
    auto resample_context_ptr = resample_context.get();

    // The channel_layout may need to be populated from the number of channels.
    // This is usually the case with formats using the pcm_16* codecs where
    // it's not nessicarily possible to determine the channel layout. In these
    // situations we can use the default channel_layout
    if ( ! codec_context->channel_layout)
    {
        codec_context->channel_layout = av_get_default_channel_layout(codec_context->channels);
    }

    av_opt_set_int(resample_context_ptr, "in_sample_fmt",      codec_context->sample_fmt,     0);
    av_opt_set_int(resample_context_ptr, "in_sample_rate",     codec_context->sample_rate,    0);
    av_opt_set_int(resample_context_ptr, "in_channel_layout",  codec_context->channel_layout, 0);
    av_opt_set_int(resample_context_ptr, "out_sample_fmt",     AV_SAMPLE_FMT_S16,             0);
    av_opt_set_int(resample_context_ptr, "out_sample_rate",    codec_context->sample_rate,    0);
    av_opt_set_int(resample_context_ptr, "out_channel_layout", codec_context->channel_layout, 0);

    if (avresample_open(resample_context_ptr) < 0)
        throw std::runtime_error("Unable to open the resample context");

    // Prepare the KeyFinder::AudioData object
    audio.setFrameRate((unsigned int) codec_context->sample_rate);
    audio.setChannels(codec_context->channels);

    SafeAVPacket packet;
    std::shared_ptr<AVFrame> audio_frame(av_frame_alloc(), &av_free);

    int current_packet_offset = 0;

    // Read all stream samples into the AudioData container
    while (true)
    {
        if (current_packet_offset >= packet.inner_packet.size)
        {
            while (true)
            {
                packet.fill_packet(format_ptr);

                // Stop reading once we've read a packet from this stream
                if (packet.inner_packet.stream_index == audio_stream->index)
                    break;
            }

            current_packet_offset = 0;

            // We're all done once we have no more packets to read
            if (packet.inner_packet.size <= 0)
                break;
        }

        int frame_available = 0;
        const auto processed_size = avcodec_decode_audio4(codec_context,
                audio_frame.get(), &frame_available, &packet.inner_packet);

        if (processed_size < 0)
            throw std::runtime_error("Unable to process the encoded audio data");

        current_packet_offset += processed_size;

        // Seek The packet forward for the ammount of data we've read. If there
        // is still data left in the packet that wasn't decoded we will handle
        // that next interation of this loop
        packet.inner_packet.size -= current_packet_offset;
        packet.inner_packet.data += current_packet_offset;

        // Not enough data to read the frame. Keep going
        if ( ! frame_available)
            continue;

        // The KeyFinder::AudioData object expects non-planar 16 bit PCM data.
        // If we didn't decode audio data in that format we have to re-sample
        if (codec_context->sample_fmt != AV_SAMPLE_FMT_S16)
        {
            std::shared_ptr<AVFrame> converted_frame(av_frame_alloc(), &av_free);

            converted_frame->channel_layout = audio_frame->channel_layout;
            converted_frame->sample_rate = audio_frame->sample_rate;
            converted_frame->format = AV_SAMPLE_FMT_S16;

            if (avresample_convert_frame(resample_context_ptr, converted_frame.get(), audio_frame.get()) < 0)
                throw std::runtime_error("Unable to resample audio into 16bit PCM data");

            audio_frame.swap(converted_frame);
        }

        // Since we we're dealing with 16bit samples we need to convert our
        // data pointer to a int16_t (from int8_t). This also means that we
        // need to halve our sample count since the sample count expected one
        // byte per sample, instead of two.
        int16_t* sample_data = (int16_t*) audio_frame->extended_data[0];
        int sample_count = audio_frame->linesize[0] / 2;

        // Populate the KeyFinder::AudioData object with the samples
        int old_sample_count = audio.getSampleCount();
        audio.addToSampleCount(sample_count);
        audio.resetIterators();
        audio.advanceWriteIterator(old_sample_count);

        for (int i = 0; i < sample_count; ++i)
        {
            audio.setSampleAtWriteIterator((float) sample_data[i]);
            audio.advanceWriteIterator();
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    // File path of the file we want to determine the key of
    const char* file_path = argv[1];

    KeyFinder::AudioData audio_data;

    fill_audio_data(file_path, audio_data);

    KeyFinder::KeyFinder key_finder;

    KeyFinder::key_t key = key_finder.keyOfAudio(audio_data).globalKeyEstimate;

    // Only return a key when we don't have silence
    // Rule 12: Be quiet!
    if (key != KeyFinder::SILENCE)
    {
        std::cout << KeyNotation::camelot[key] << std::endl;
    }

    return 0;
}
