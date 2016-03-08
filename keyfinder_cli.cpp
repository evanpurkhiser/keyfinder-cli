#include <iostream>
#include <mutex>
#include <memory>
#include <getopt.h>
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

const int BAD_PACKET_THRESHOLD = 100;

/**
 * The "safe" AVPacket wrapper will handle memory management of the packet,
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
            av_packet_unref(&inner_packet);
        }
    }

    /**
     * Read into this packet from the format_context. A stream index should
     * also be provided so that the packet knows what stream to read from.
     *
     * @param format_context The format context to read data from
     * @param stream_index   The index of the stream we want data from
     */
    void read(AVFormatContext* format_context, int stream_index)
    {
        while (true)
        {
            if (inner_packet.data)
            {
                av_packet_unref(&inner_packet);
            }

            if (av_read_frame(format_context, &inner_packet) < 0)
            {
                inner_packet.data = nullptr;
            }

            // Stop reading once we've read a packet from this stream
            if (inner_packet.stream_index == stream_index)
                break;
        }
    }
};

/**
 * Fill an instance of KeyFinder::AudioData with the audio data from a file.
 * This does the ffmpeg dance to decode any type of audio stream into PCM_16
 * samples.
 *
 * @param file_path The file to read audio data from
 * @param audio     The KeyFinder AudioData container to fill
 */
void fill_audio_data(const char* file_path, KeyFinder::AudioData &audio)
{
    // Initialize AV format/codec things once
    static std::once_flag init_flag;
    std::call_once(init_flag, []() { av_register_all(); });

    AVFormatContext* format_ctx_ptr = avformat_alloc_context();

    // Open the file for decoding
    if (avformat_open_input(&format_ctx_ptr, file_path, nullptr, nullptr) < 0)
        throw std::runtime_error("Unable to open audio file (File doesn't eixst or unhandle format)");

    // Manage the format context. Instead of initalizing this before opening
    // the input we handle it after since avformat_open_input will free the
    // context for us upon error.
    std::shared_ptr<AVFormatContext> format_context(format_ctx_ptr, &avformat_free_context);

    // Determine stream information
    if (avformat_find_stream_info(format_ctx_ptr, nullptr) < 0)
        throw std::runtime_error("Unable to get stream info");

    const AVStream* audio_stream = nullptr;

    // Get the audio stream from the context. We have to look through all of
    // the streams, we pick the first one that is recognized as an audio stream
    for (unsigned int i = 0; i < format_context->nb_streams; ++i)
    {
        auto stream = format_context->streams[i];

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
    auto resample_ctx_ptr = resample_context.get();

    // The channel_layout may need to be populated from the number of channels.
    // This is usually the case with formats using the pcm_16* codecs where
    // it's not nessicarily possible to determine the channel layout. In these
    // situations we can use the default channel_layout
    if ( ! codec_context->channel_layout)
    {
        codec_context->channel_layout = av_get_default_channel_layout(codec_context->channels);
    }

    av_opt_set_int(resample_ctx_ptr, "in_sample_fmt",      codec_context->sample_fmt,     0);
    av_opt_set_int(resample_ctx_ptr, "in_sample_rate",     codec_context->sample_rate,    0);
    av_opt_set_int(resample_ctx_ptr, "in_channel_layout",  codec_context->channel_layout, 0);
    av_opt_set_int(resample_ctx_ptr, "out_sample_fmt",     AV_SAMPLE_FMT_S16,             0);
    av_opt_set_int(resample_ctx_ptr, "out_sample_rate",    codec_context->sample_rate,    0);
    av_opt_set_int(resample_ctx_ptr, "out_channel_layout", codec_context->channel_layout, 0);

    if (avresample_open(resample_ctx_ptr) < 0)
        throw std::runtime_error("Unable to open the resample context");

    // Prepare the KeyFinder::AudioData object
    audio.setFrameRate((unsigned int) codec_context->sample_rate);
    audio.setChannels(codec_context->channels);

    SafeAVPacket packet;
    std::shared_ptr<AVFrame> audio_frame(av_frame_alloc(), &av_free);

    int current_packet_offset = 0;
    int back_packet_count = 0;

    // Read all stream samples into the AudioData container
    while (true)
    {
        // Read another packet once we've consumed all of the previous one
        if (current_packet_offset >= packet.inner_packet.size)
        {
            packet.read(format_ctx_ptr, audio_stream->index);

            current_packet_offset = 0;

            // We're all done once we have no more packets to read
            if (packet.inner_packet.size <= 0)
                break;
        }

        int frame_available = 0;
        const auto processed_size = avcodec_decode_audio4(codec_context,
                audio_frame.get(), &frame_available, &packet.inner_packet);

        // Bad packet. Maybe we can ignore it
        if (processed_size < 0)
        {
            if (++back_packet_count > BAD_PACKET_THRESHOLD)
                throw std::runtime_error("Too many bad packets");

            current_packet_offset    = 0;
            packet.inner_packet.size = 0;

            continue;
        }

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

            if (avresample_convert_frame(resample_ctx_ptr, converted_frame.get(), audio_frame.get()) < 0)
                throw std::runtime_error("Unable to resample audio into 16bit PCM data");

            audio_frame.swap(converted_frame);
        }

        // Since we we're dealing with 16bit samples we need to convert our
        // data pointer to a int16_t (from int8_t). This also means that we
        // need to halve our sample count since the sample count expected one
        // byte per sample, instead of two.
        int16_t* sample_data = (int16_t*) audio_frame->extended_data[0];
        unsigned int sample_count = audio_frame->linesize[0] / 2;

        // Populate the KeyFinder::AudioData object with the samples
        int old_sample_count = audio.getSampleCount();
        audio.addToSampleCount(sample_count);
        audio.resetIterators();
        audio.advanceWriteIterator(old_sample_count);

        for (unsigned int i = 0; i < sample_count; ++i)
        {
            audio.setSampleAtWriteIterator((float) sample_data[i]);
            audio.advanceWriteIterator();
        }
    }
}

int main(int argc, char** argv)
{
    auto display_usage = [argv](std::ostream &stream)
    {
        stream << "Usage: " << argv[0] << " [-h] [-n key-notation] filename"
               << std::endl;
    };

    // Default to the standard key notation
    auto selected_notation = KeyNotation::standard;

    struct option options[] =
    {
        {"notation", required_argument, 0, 'n'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    opterr = 0;

    char c;
    while ((c = getopt_long(argc, argv, "n:h", options, nullptr)) != -1)
    {
        switch (c)
        {
        case 'h':
            display_usage(std::cout);
            return 0;

        case '?':
            display_usage(std::cerr);
            return 0;

        case 'n':
            if (KeyNotation::mappings.find(optarg) == KeyNotation::mappings.end())
            {
                std::cerr << "Invalid key notation" << std::endl;
                return 1;
            }

            selected_notation = KeyNotation::mappings[optarg];
            break;
        }
    }

    // There should be at least one argument left for the filename. We can
    // check for this by seeing if the opt parser's last index is larger than
    // the arg count.
    if (optind >= argc)
    {
        display_usage(std::cerr);
        return 1;
    }

    char* file_path = argv[optind];

    KeyFinder::KeyFinder key_finder;
    KeyFinder::AudioData audio_data;
    KeyFinder::key_t key;

    // Hide av* warnings and errors
    av_log_set_callback([](void *, int, const char*, va_list) {});

    try
    {
        fill_audio_data(file_path, audio_data);
        key = key_finder.keyOfAudio(audio_data);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Only return a key when we don't have silence - rule 12: Be quiet!
    if (key != KeyFinder::SILENCE)
    {
        std::cout << selected_notation[key] << std::endl;
    }

    return 0;
}
