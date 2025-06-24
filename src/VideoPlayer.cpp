
#include "../include/VideoPlayer.hpp"
#include <Geode/Bindings.hpp>
#include <Geode/Geode.hpp>
#include <math.h>
using namespace geode::prelude;

#define start_video_stream(filename, fmt_ctx, codec_ctx, codec, stream_index)               \
    do {                                                                                    \
        if (avformat_open_input(&(fmt_ctx), (filename), NULL, NULL) < 0) {                 \
            log::error("Could not open input file: {}", filename);                \
            break;                                                                          \
        }                                                                                   \
        if (avformat_find_stream_info((fmt_ctx), NULL) < 0) {                               \
            log::error("Failed to get stream info");                                 \
            break;                                                                          \
        }                                                                                   \
        (stream_index) = av_find_best_stream((fmt_ctx), AVMEDIA_TYPE_VIDEO, -1, -1, &(codec), 0); \
        if ((stream_index) < 0) {                                                           \
            log::error("Could not find a video stream");                             \
            break;                                                                          \
        }                                                                                   \
        (codec_ctx) = avcodec_alloc_context3((codec));                                      \
        avcodec_parameters_to_context((codec_ctx), (fmt_ctx)->streams[(stream_index)]->codecpar); \
        if (avcodec_open2((codec_ctx), (codec), NULL) < 0) {                                \
            log::error("Could not open codec");                                      \
            break;                                                                          \
        }                                                                                   \
    } while (0)


#define APP_SHADER_SOURCE(...) #__VA_ARGS__

const char* APP_VERTEX_SHADER = APP_SHADER_SOURCE(
    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    varying vec2 tex_coord;

	void main() {
        tex_coord = a_texCoord;
		gl_Position = CC_MVPMatrix * a_position;
	}
);

const char* APP_FRAGMENT_SHADER_YCRCB = APP_SHADER_SOURCE(
	uniform sampler2D texture_y;
	uniform sampler2D texture_cb;
	uniform sampler2D texture_cr;
    varying vec2 tex_coord;

	mat4 rec601 = mat4(
		1.16438,  0.00000,  1.59603, -0.87079,
		1.16438, -0.39176, -0.81297,  0.52959,
		1.16438,  2.01723,  0.00000, -1.08139,
		0, 0, 0, 1
	);

	void main() {
		float y = texture2D(texture_y, tex_coord).r;
		float cb = texture2D(texture_cb, tex_coord).r;
		float cr = texture2D(texture_cr, tex_coord).r;

		gl_FragColor = vec4(y, cb, cr, 1.0) * rec601;
	}
);

namespace videoplayer {
    bool VideoPlayer::init(std::filesystem::path const& path, bool loop) {
        if (!CCNode::init()) return false;

        // GENERAL
        m_path = path;
        start_video_stream(m_path.string().c_str(), m_fmt_ctx, m_codec_ctx, m_codec, m_video_stream_index);
        
        /*if (!m_stream) {
            log::error("File at {} not found", m_path.string());
            return false;
        };
        if (!m_stream->video_decoder){
            log::error("{} is not a valid video", m_path.string());
            return false;
        }

        //plm_set_loop(m_stream, loop);
        m_loop = loop;

<<<<<<< HEAD
        //plm_set_video_decode_callback(m_stream, VideoPlayer::videoCallback, this);
        //plm_set_audio_decode_callback(m_stream, VideoPlayer::audioCallback, this);
        */
=======
        this->m_maxTime = plm_get_duration(m_stream);

        plm_set_video_decode_callback(m_stream, VideoPlayer::videoCallback, this);
        plm_set_audio_decode_callback(m_stream, VideoPlayer::audioCallback, this);
>>>>>>> accb4f8ea2fce88ad87eb75162372ed6e2a170c4

        // VIDEO
        int height = m_fmt_ctx->streams[m_video_stream_index]->codecpar->height;
        int width = m_fmt_ctx->streams[m_video_stream_index]->codecpar->width;

        m_dimensions = CCSize(width, height);

        CCGLProgram* shader = new CCGLProgram;

        setContentSize(m_dimensions * 4);
        shader->initWithVertexShaderByteArray(APP_VERTEX_SHADER, APP_FRAGMENT_SHADER_YCRCB);

        shader->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
        shader->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);

        shader->link();
        shader->updateUniforms();

        const char* texture_names[3] = {"texture_y", "texture_cb", "texture_cr"};

       // plm_frame_t* frame = &m_stream->video_decoder->frame_current;
       // plm_plane_t planes[3] = {frame->y, frame->cb, frame->cr};

        for (int i = 0; i < 3; i++) {
            GLuint texture;
            glGenTextures(1, &texture);

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glUniform1i(glGetUniformLocation(shader->getProgram(), texture_names[i]), i);

            m_textures[i] = texture;
        }

        setShaderProgram(shader);
        shader->release();
        getShaderProgram()->use();

        // Audio
        initAudio();

        m_paused = false;
        setAnchorPoint({0.5f, 0.5f});
        scheduleUpdate();

        CCTexture2D* tex = new CCTexture2D;
        tex->autorelease();

        CCSprite* overdrawFix = CCSprite::createWithTexture(tex);
        overdrawFix->setID("overdraw-fix");
        this->addChild(overdrawFix);

        return true;
    };

    void VideoPlayer::initAudio() {
        FMODAudioEngine* engine = FMODAudioEngine::sharedEngine();

        int samples_per_frame = m_codec_ctx->frame_size;
        int channels = m_codec_ctx->channels;
        int bytes_per_sample = av_get_bytes_per_sample(m_codec_ctx->sample_fmt);
        double duration_seconds = (double)m_fmt_ctx->duration / AV_TIME_BASE;
        int decode_buffer_size = samples_per_frame * channels * bytes_per_sample;

        FMOD_CREATESOUNDEXINFO soundInfo;
        memset(&soundInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        soundInfo.decodebuffersize = decode_buffer_size;
        soundInfo.length = (unsigned int)(m_codec_ctx->sample_rate * channels * bytes_per_sample * duration_seconds);
        soundInfo.numchannels = 2;
        soundInfo.defaultfrequency = m_codec_ctx->sample_rate;
        soundInfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
        soundInfo.pcmreadcallback = &VideoPlayer::PCMRead;
        soundInfo.userdata = this;

        m_samples = {};
        engine->m_system->createStream(nullptr, FMOD_OPENUSER, &soundInfo, &m_sound);

        FMOD::ChannelGroup* group;

        FMODAudioEngine::sharedEngine()->m_system->getMasterChannelGroup(&group);
        //group = FMODAudioEngine::sharedEngine()->m_globalChannel;
		//FMODAudioEngine::sharedEngine()->m_system->createChannelGroup("Fix_Audio", &group);
        engine->m_system->playSound(m_sound, group, false, &m_channel);
        m_channel->setVolume(m_volume);
        
        m_channel->setUserData(this);
        if (m_loop) m_channel->setCallback(&VideoPlayer::audioCallback);
    }

    FMOD_RESULT F_CALLBACK VideoPlayer::audioCallback(FMOD_CHANNELCONTROL *chanControl, FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2) {
        if (callbackType != FMOD_CHANNELCONTROL_CALLBACK_END) return FMOD_OK;

        VideoPlayer* self;
        ((FMOD::ChannelControl*)chanControl)->getUserData((void**)&self);
        if (self->m_stopped) return FMOD_OK; // For destructor/onExit
        self->m_channel->stop();
        self->m_sound->release();

        self->initAudio();
        return FMOD_OK;
    }

    static int times = 0;
    void VideoPlayer::update(float delta) {
<<<<<<< HEAD
        if (!m_paused) {}; //plm_decode(m_stream, delta);
=======
        if (!m_paused) {
            plm_decode(m_stream, delta);
            m_currentTime = plm_get_time(m_stream);

            if (!m_loop && m_currentTime >= m_maxTime) {
                if (m_onVideoEnd) { 
                    m_onVideoEnd(); // Trigger callback
                    m_onVideoEnd = nullptr;
                }
            }
        }
    }

    double VideoPlayer::getMaxTime() const {
        return m_maxTime;
    }

    double VideoPlayer::getCurrentTime() const {
        return m_currentTime;
    }

    void VideoPlayer::onVideoEnd(std::function<void()> callback) {
        m_onVideoEnd = std::move(callback);
>>>>>>> accb4f8ea2fce88ad87eb75162372ed6e2a170c4
    }

    void VideoPlayer::draw() {
        CC_NODE_DRAW_SETUP();

        for (int i = 0; i < 3; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        }

        float w = m_obContentSize.width;
        float h = m_obContentSize.height;

        GLfloat vertices[12] = {0,0, w,0, w,h, 0,0, 0,h, w,h};
        GLfloat coordinates[12] = {0,1, 1,1, 1,0, 0,1, 0,0, 1,0};


        glEnableVertexAttribArray(kCCVertexAttribFlag_Position);
        glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        
        glEnableVertexAttribArray(kCCVertexAttribFlag_TexCoords);
        glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, coordinates);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void VideoPlayer::onExit() {
        m_channel->stop();
        m_sound->release();
        m_stopped = true;
    }

    VideoPlayer::~VideoPlayer() {
        onExit();
    }

    /*void VideoPlayer::videoCallback(plm_t* mpeg, plm_frame_t* frame, void* user) {
        VideoPlayer* self = (VideoPlayer*) user;

        plm_plane_t* frames[3] = {&frame->y, &frame->cb, &frame->cr};

        for (int i = 0; i < 3; i++) {
            GLuint texture = self->m_textures[i];

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_LUMINANCE, frames[i]->width, frames[i]->height, 0,
                GL_LUMINANCE, GL_UNSIGNED_BYTE, frames[i]->data
            );
        }
    }

    void VideoPlayer::audioCallback(plm_t* mpeg, plm_samples_t* samples, void* user) {
        VideoPlayer* self = (VideoPlayer*) user;

        for (unsigned int i = 0; i < samples->count * 2; i++) {
            self->m_samples.push(samples->interleaved[i]);
        }

        while (self->m_samples.size() > PLM_AUDIO_SAMPLES_PER_FRAME * 16) { // i think this is 4 frames of wiggle room but im not sure it just sounds best this way
            self->m_samples.pop();
        }
    }*/

    FMOD_RESULT F_CALLBACK VideoPlayer::PCMRead(FMOD_SOUND *sound, void *data, unsigned int length) {
        VideoPlayer* self;
        ((FMOD::Sound*)sound)->getUserData((void**)&self);
        if (!self) return FMOD_OK;

        float* buf = (float*)data;

        for (unsigned int i = 0; i < (length / sizeof(float)) / 2 && self->m_samples.size() >= 2; i++) { // Always keep the ears synced
            buf[2*i]=self->m_samples.front();
            self->m_samples.pop();

            buf[2*i+1]=self->m_samples.front();
            self->m_samples.pop();
        }

        return FMOD_OK;
    }

    VideoPlayer* VideoPlayer::create(std::filesystem::path const& path, bool loop) {
        VideoPlayer* ret = new VideoPlayer;
        if (ret && ret->init(path, loop)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    };

    void VideoPlayer::setHeight(float height) {
        setContentSize({height * m_dimensions.aspect(), height});
    }

    void VideoPlayer::setWidth(float width) {
        setContentSize({width, width / m_dimensions.aspect()});
    }

    void VideoPlayer::fillSize(CCSize size) {
        if (m_dimensions.aspect() > size.aspect()) {
            setWidth(size.width);
        } else {
            setHeight(size.height);
        }
    }

    void VideoPlayer::setVolume(float volume) {
        m_volume = volume;
        m_channel->setVolume(volume);
    }

    void VideoPlayer::pause() {
        m_channel->setPaused(true);
        m_paused = true;
    }

    void VideoPlayer::resume() {
        m_channel->setPaused(false);
        m_paused = false;
    }

    void VideoPlayer::toggle() {
        if (m_paused) return resume();
        pause();
    }

     bool VideoPlayer::isPaused() {
        return m_paused;
    }
}
