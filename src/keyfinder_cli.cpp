#include <iostream>
#include <mutex>
#include <keyfinder/keyfinder.h>

extern "C"{
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    // File path of the file we want to determine the key of
    char* file_path = argv[1];

    // Initalize AV format/codec things once
    static std::once_flag init_flag;
    std::call_once(init_flag, []() { av_register_all(); });

    // Setup the avFormat context pointer
    std::shared_ptr<AVFormatContext> av_format(avformat_alloc_context(), &avformat_free_context);
    auto av_format_ptr = av_format.get();

    // Open the file for decoding
    if (avformat_open_input(&av_format_ptr, file_path, nullptr, nullptr) < 0)
        throw std::runtime_error("Unable to load media file (probably invalid format).");

    // Deterine stream information
    if (avformat_find_stream_info(av_format_ptr, nullptr) < 0)
        throw std::runtime_error("Unable to get stream info.");

    // Get the audio stream from the context
    AVStream* audio_stream = nullptr;

    for (unsigned int i = 0; i < av_format->nb_streams; ++i)
    {
        auto stream = av_format->streams[i];

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

    // Setup the KeyFinder AudiooData object
    KeyFinder::AudioData audio_data;

    audio_data.setFrameRate((unsigned int) codec_context->sample_rate);
    audio_data.setChannels(codec_context->channels);

    return 0;
}
