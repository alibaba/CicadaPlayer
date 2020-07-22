//
// Created by moqi on 2020/7/20.
//

#include "AppleAVPlayer.h"
using namespace Cicada;

AppleAVPlayer AppleAVPlayer::se(1);
AppleAVPlayer::AppleAVPlayer()
{}
AppleAVPlayer::~AppleAVPlayer()
{}
int AppleAVPlayer::SetListener(const playerListener &Listener)
{
    return 0;
}
void AppleAVPlayer::SetOnRenderCallBack(onRenderFrame cb, void *userData)
{}
void AppleAVPlayer::SetView(void *view)
{}
void AppleAVPlayer::SetDataSource(const char *url)
{}
void AppleAVPlayer::Prepare()
{}
void AppleAVPlayer::Start()
{}
void AppleAVPlayer::Pause()
{}
StreamType AppleAVPlayer::SwitchStream(int index)
{
    return ST_TYPE_AUDIO;
}
void AppleAVPlayer::SeekTo(int64_t seekPos, bool bAccurate)
{}
int AppleAVPlayer::Stop()
{
    return 0;
}
PlayerStatus AppleAVPlayer::GetPlayerStatus() const
{
    return PLAYER_STOPPED;
}
int64_t AppleAVPlayer::GetDuration() const
{
    return 0;
}
int64_t AppleAVPlayer::GetPlayingPosition()
{
    return 0;
}
int64_t AppleAVPlayer::GetBufferPosition()
{
    return 0;
}
void AppleAVPlayer::Mute(bool bMute)
{}
bool AppleAVPlayer::IsMute() const
{
    return false;
}
void AppleAVPlayer::SetVolume(float volume)
{}
float AppleAVPlayer::GetVideoRenderFps()
{
    return 0;
}
void AppleAVPlayer::EnterBackGround(bool back)
{}
void AppleAVPlayer::SetScaleMode(ScaleMode mode)
{}
ScaleMode AppleAVPlayer::GetScaleMode()
{
    return SM_FIT;
}
void AppleAVPlayer::SetRotateMode(RotateMode mode)
{}
RotateMode AppleAVPlayer::GetRotateMode()
{
    return ROTATE_MODE_180;
}
void AppleAVPlayer::SetMirrorMode(MirrorMode mode)
{}
void AppleAVPlayer::SetVideoBackgroundColor(uint32_t color)
{}
MirrorMode AppleAVPlayer::GetMirrorMode()
{
    return MIRROR_MODE_NONE;
}
int AppleAVPlayer::GetCurrentStreamIndex(StreamType type)
{
    return 0;
}
StreamInfo *AppleAVPlayer::GetCurrentStreamInfo(StreamType type)
{
    return nullptr;
}
int64_t AppleAVPlayer::GetMasterClockPts()
{
    return 0;
}
void AppleAVPlayer::SetTimeout(int timeout)
{}
void AppleAVPlayer::SetDropBufferThreshold(int dropValue)
{}
void AppleAVPlayer::SetDecoderType(DecoderType type)
{}
DecoderType AppleAVPlayer::GetDecoderType()
{
    return DT_SOFTWARE;
}
float AppleAVPlayer::GetVolume() const
{
    return 0;
}
void AppleAVPlayer::SetRefer(const char *refer)
{}
void AppleAVPlayer::SetUserAgent(const char *userAgent)
{}
void AppleAVPlayer::SetLooping(bool bCirclePlay)
{}
bool AppleAVPlayer::isLooping()
{
    return false;
}
void AppleAVPlayer::CaptureScreen()
{}
void AppleAVPlayer::GetVideoResolution(int &width, int &height)
{}
void AppleAVPlayer::GetVideoRotation(int &rotation)
{}
std::string AppleAVPlayer::GetPropertyString(PropertyKey key)
{
    return ICicadaPlayer::GetPropertyString(key);
}
int64_t AppleAVPlayer::GetPropertyInt(PropertyKey key)
{
    return ICicadaPlayer::GetPropertyInt(key);
}
float AppleAVPlayer::GetVideoDecodeFps()
{
    return 0;
}
int AppleAVPlayer::SetOption(const char *key, const char *value)
{
    return 0;
}
void AppleAVPlayer::GetOption(const char *key, char *value)
{}
void AppleAVPlayer::setSpeed(float speed)
{}
float AppleAVPlayer::getSpeed()
{
    return 0;
}
void AppleAVPlayer::AddCustomHttpHeader(const char *httpHeader)
{}
void AppleAVPlayer::RemoveAllCustomHttpHeader()
{}
void AppleAVPlayer::addExtSubtitle(const char *uri)
{}
int AppleAVPlayer::selectExtSubtitle(int index, bool bSelect)
{
    return 0;
}
int AppleAVPlayer::getCurrentStreamMeta(Stream_meta *meta, StreamType type)
{
    return 0;
}
void AppleAVPlayer::reLoad()
{}
void AppleAVPlayer::SetAutoPlay(bool bAutoPlay)
{}
bool AppleAVPlayer::IsAutoPlay()
{
    return false;
}
int AppleAVPlayer::invokeComponent(std::string content)
{
    return 0;
}
