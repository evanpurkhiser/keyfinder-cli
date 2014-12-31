#include <iostream>
#include <mutex>
#include <keyfinder/keyfinder.h>
#include <keyfinder/constants.h>

extern "C"{
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

std::map<KeyFinder::key_t, std::string> key_map =
{
    {KeyFinder::A_MAJOR,       "A" }, {KeyFinder::A_MINOR,       "Am" },
    {KeyFinder::B_FLAT_MAJOR,  "B" }, {KeyFinder::B_FLAT_MINOR,  "Bbm"},
    {KeyFinder::B_MAJOR,       "B" }, {KeyFinder::B_MINOR,       "Bm" },
    {KeyFinder::C_MAJOR,       "C" }, {KeyFinder::C_MINOR,       "Cm" },
    {KeyFinder::D_FLAT_MAJOR,  "Db"}, {KeyFinder::D_FLAT_MINOR,  "Dbm"},
    {KeyFinder::D_MAJOR,       "D" }, {KeyFinder::D_MINOR,       "Dm" },
    {KeyFinder::E_FLAT_MAJOR,  "Eb"}, {KeyFinder::E_FLAT_MINOR,  "Ebm"},
    {KeyFinder::E_MAJOR,       "E" }, {KeyFinder::E_MINOR,       "Em" },
    {KeyFinder::F_MAJOR,       "F" }, {KeyFinder::F_MINOR,       "Em" },
    {KeyFinder::G_FLAT_MAJOR,  "Gb"}, {KeyFinder::G_FLAT_MINOR,  "Gbm"},
    {KeyFinder::G_MAJOR,       "G" }, {KeyFinder::G_MINOR,       "Gm" },
    {KeyFinder::A_FLAT_MAJOR,  "Ab"}, {KeyFinder::A_FLAT_MINOR,  "Abm"},
    {KeyFinder::SILENCE,   "Silent"},
};

std::map<KeyFinder::key_t, std::string> camelot_map =
{
    {KeyFinder::A_MAJOR,       "11B"}, {KeyFinder::A_MINOR,       "8A" },
    {KeyFinder::B_FLAT_MAJOR,  "6B" }, {KeyFinder::B_FLAT_MINOR,  "3A" },
    {KeyFinder::B_MAJOR,       "1B" }, {KeyFinder::B_MINOR,       "10A"},
    {KeyFinder::C_MAJOR,       "8B" }, {KeyFinder::C_MINOR,       "5A" },
    {KeyFinder::D_FLAT_MAJOR,  "3B" }, {KeyFinder::D_FLAT_MINOR,  "12A"},
    {KeyFinder::D_MAJOR,       "10B"}, {KeyFinder::D_MINOR,       "7A" },
    {KeyFinder::E_FLAT_MAJOR,  "5B" }, {KeyFinder::E_FLAT_MINOR,  "2A" },
    {KeyFinder::E_MAJOR,       "12B"}, {KeyFinder::E_MINOR,       "9A" },
    {KeyFinder::F_MAJOR,       "7B" }, {KeyFinder::F_MINOR,       "4A" },
    {KeyFinder::G_FLAT_MAJOR,  "2B" }, {KeyFinder::G_FLAT_MINOR,  "11A"},
    {KeyFinder::G_MAJOR,       "9B" }, {KeyFinder::G_MINOR,       "6A" },
    {KeyFinder::A_FLAT_MAJOR,  "4B" }, {KeyFinder::A_FLAT_MINOR,  "1A" },
    {KeyFinder::SILENCE,    "Silent"},
};

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

    // Setup the KeyFinder AudioData object
    KeyFinder::AudioData audio;

    audio.setFrameRate((unsigned int) codec_context->sample_rate);
    audio.setChannels(codec_context->channels);

    // Error checking from this point on becomes... lax

    AVPacket packet;
    packet.size = 0;
    int current_packet_offset = 0;
    std::shared_ptr<AVFrame> av_frame(av_frame_alloc(), &av_free);

    // Read all stream samples into the AudioData container
    while (true)
    {
        // Only read packets if we need to
        if (current_packet_offset >= packet.size)
        {
            // Read in a single packet until we no longer can
            if (av_read_frame(av_format_ptr, &packet) < 0)
                break;

            // Ignore packets from other streams
            if (packet.stream_index != audio_stream->index)
                continue;

            current_packet_offset = 0;
        }

        if (current_packet_offset > 0)
            std::cout << "skipping read!" << std::endl;




        int frame_available = 0;
        const auto processed_size = avcodec_decode_audio4(codec_context,
                av_frame.get(), &frame_available, &packet);

        if (!frame_available)
            continue;

        if (processed_size < 0)
            throw std::runtime_error("Unable to process the encoded audio data");

        current_packet_offset += processed_size;

        packet.size -= processed_size;
        packet.data += processed_size;

        if (codec_context->sample_fmt != AV_SAMPLE_FMT_S16)
            throw std::runtime_error("Doesn't handle resampling yet");

        int16_t* sample_data = (int16_t *) av_frame->data[0];

        int oldSampleCount = audio.getSampleCount();
        audio.addToSampleCount(av_frame->linesize[0]*2);
        audio.resetIterators();
        audio.advanceWriteIterator(oldSampleCount);

        for (int i = 0; i < av_frame->linesize[0]*2; ++i)
        {
            audio.setSampleAtWriteIterator((float) sample_data[i]);
            audio.advanceWriteIterator();
        }
    }

    // Setup keyfinder to process the AudioData
    KeyFinder::KeyFinder key_finder;

    KeyFinder::key_t result = key_finder.keyOfAudio(audio).globalKeyEstimate;

    std::cout << key_map[result] << std::endl;

    return 0;
}
