#ifndef _VIDEO_PLAYER_HPP
#define _VIDEO_PLAYER_HPP

#ifdef GEODE_IS_WINDOWS
    #ifdef _VIDEO_PLAYER_EXPORTING
        #define VIDEO_PLAYER_DLL __declspec(dllexport)
    #else
        #define VIDEO_PLAYER_DLL __declspec(dllimport)
    #endif
#else
    #define VIDEO_PLAYER_DLL
#endif


#include <Geode/Bindings.hpp>
#include <Geode/Geode.hpp>
#include <Geode/cocos/platform/CCGL.h>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/avutil.h>
}

#include <queue>

typedef unsigned int GLuint;
using namespace geode::prelude;

namespace videoplayer {
    class VIDEO_PLAYER_DLL VideoPlayer : public cocos2d::CCNodeRGBA {
    protected:
        bool init(std::filesystem::path const& path, bool loop);
        
        void initAudio();
        static FMOD_RESULT F_CALLBACK audioCallback(FMOD_CHANNELCONTROL *chanControl, FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);

        virtual void update(float delta) override;
        virtual void draw() override;

        virtual ~VideoPlayer();
        virtual void onExit() override;

        static FMOD_RESULT F_CALLBACK PCMRead(FMOD_SOUND *sound, void *data, unsigned int length);

        void decodeFrame();

        double m_lastframe = 0.0;
        double m_framerate = 0;
        double m_frameTime = 60.0;

        std::filesystem::path m_path;

        const AVCodec * m_codec = NULL;
        AVFormatContext * m_fmt_ctx = NULL;
        AVCodecContext * m_codec_ctx = NULL;
        int m_video_stream_index = -1;

        const AVCodec * m_audio_codec = NULL;
        AVCodecContext* m_audio_codec_ctx = NULL;
        int m_audio_stream_index = -1;

        cocos2d::CCSize m_dimensions;
        GLuint m_textures[3]; // y, cb, cr
        std::queue<float> m_samples;

        FMOD::Channel* m_channel;
        FMOD::Sound* m_sound;

        bool m_paused;
        bool m_loop;
        bool m_stopped;
        float m_volume = 1.0f;
        double m_currentTime = 0.0;
        double m_maxTime = 1.0;
        std::function<void()> m_onVideoEnd = nullptr;

    public:

        /**
         * @brief Allocates and initializes a video player.
         * 
         * @param path Path to the video file (Could use Mod::get()->getResourcesDir() / "file.mpg").
         * @param loop Whether or not playback should loop upon completion.
         * @return A new initialized video player
         */
        static VideoPlayer* create(std::filesystem::path const& path, bool loop=false);

        /**
         * @brief Sets the content height of the video player, maintaining aspect ratio
         * 
         * @param height The new content height for the video player
         */
        void setHeight(float height);

        /**
         * @brief Sets the content width of the video player, maintaining aspect ratio
         * 
         * @param width The new content width for the video player
         */
        void setWidth(float width);

        /**
         * @brief Modifies the content size of the video player to fit a given size, maintaining aspect ratio
         * 
         * @param size The size to fill
         */
        void fillSize(cocos2d::CCSize size);

        /**
         * @brief Sets the volume of playback.
         * 
         * @param volume The new volume
         */
        void setVolume(float volume);

        /**
         * @brief Pauses playback
         * 
         */
        void pause();

        /**
         * @brief Resumes playback
         * 
         */
        void resume();

        /**
         * @brief Toggles playback.
         * 
         */
        void toggle();

        /**
         * @brief Returns whether playback is paused.
         * 
         * @return The playback status
         */
        bool isPaused();
         /**
         * @brief Returns the max play time of playback
         * 
         * @return The playback max time
         */
        double getMaxTime() const;
        /**
         * @brief Returns the current time of playback
         * 
         * @return The playback current time
         */
        double getCurrentTime() const;

         /**
         * @brief set the ending callback when the video finishes
         * 
         * @param callback The function to run
         */
        void onVideoEnd(std::function<void()> callback);

    };
}

#endif