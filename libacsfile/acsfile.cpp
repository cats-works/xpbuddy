// libacsfile - Authored in 2025 by ~cat - SOSUMI BONZIBROS
// The code is Public Domain

#include "acsfile.h"
#include "acs_private.h"

using namespace libacsfile;
using namespace std;

Character::~Character()
{
#ifdef DEBUG
    cerr << "~C";
#endif
    if(p)
        delete p;
}

bool Character::Load(const string& filename)
{
    try {
        p = new CharacterPrivate(filename);
    }
    catch(exception e) {
        return false;
    }

    if(p->acsValid)
        return true;

    return false;
}

bool Character::Loaded()
{
    if(!p)
        return false;

    return p->acsValid;
}

string Character::GUID() const
{
    if(!p)
        return "";

    return p->GuidToString(p->CharacterID);
}

string Character::Name() const
{
    if(!p)
        return "";

    return p->CharacterName;
}

string Character::Description() const
{
    if(!p)
        return "";

    return p->CharacterDescription;
}

uint16_t Character::Width() const
{
    if(!p)
        return 0;

    return p->CharacterWidth;
}

uint16_t Character::Height() const
{
    if(!p)
        return 0;

    return p->CharacterHeight;
}

bool Character::TTSEnabled() const
{
    if(!p)
        return false;

    return (bool)(p->Flags & CHAR_STYLE_TTS);
}

string Character::TTSModeGUID() const
{
    if(!p)
        return "";

    return p->GuidToString(p->ModeID);
}

string Character::TTSEngineGUID() const
{
    if(!p)
        return "";

    return p->GuidToString(p->EngineID);
}

uint32_t Character::VoiceSpeed() const
{
    if(!p)
        return 0;

    return p->Speed;
}

uint16_t Character::VoicePitch() const
{
    if(!p)
        return 0;

    return p->Pitch;
}

uint16_t Character::Gender() const
{
    if(!p)
        return 0;

    return p->Gender;
}

uint16_t Character::Age() const
{
    if(!p)
        return 0;

    return p->Age;
}

RGBQUAD Character::TransparentColor() const
{
    return p->Palette.at(p->TransparentColorIndex);
}

std::vector<RGBQUAD> Character::ColorPalette() const
{
    return p->Palette;
}

string Character::Style() const
{
    if(!p)
        return "";

    return p->Style;
}

bool Character::BalloonEnabled() const
{
    return (bool)(p->Flags & CHAR_STYLE_BALLOON);
}

Animation *Character::GetAnimation(std::string &name)
{
    if(Animations().find(name) != Animations().end())
        return Animations()[name];

    return nullptr;
}

bool Character::HasAnimation(const std::string &name)
{
    if(Animations().find(name) != Animations().end())
        return true;

    return false;
}

bool Character::HasState(const std::string &state)
{
    if(States().find(state) != States().end())
        return true;

    return false;
}

string Character::BalloonFont() const
{
    if(!p)
        return "";

    return p->FontName;
}

std::map<std::string, std::vector<std::string>> Character::States() const
{
    if(!p)
        return std::map<std::string, std::vector<std::string>>();

    return p->States;
}

vector<string> Character::AnimationNames() const
{
    vector<string> buffer;
    buffer.reserve(p->animations.size());
    for(std::map<std::string, Animation*>::iterator it = p->animations.begin();
         it != p->animations.end();
         ++it)
    {
        buffer.push_back(it->first);
    }

    return buffer;
}

map<string, Animation *> Character::Animations() const
{
    return p->animations;
}

std::map<uint16_t, Image *> Character::Images() const
{
    return p->images;
}

std::map<uint16_t, Sound *> Character::Sounds() const
{
    return p->sounds;
}

string Animation::Name() const
{
    return p->Name;
}

Animation::TransitionType Animation::Transition() const
{
    return p->Transition;
}

string Animation::ReturnAnimation() const
{
    return p->ReturnAnimation;
}

map<uint16_t, Frame*> Animation::Frames() const
{
    return p->Frames;
}

Animation::Animation(AnimationPrivate *priv)
    :p(priv) {}

Animation::~Animation()
{
#ifdef DEBUG
    cerr << "~A";
#endif
    delete p;
}

uint16_t Frame::AudioIndex() const
{
    return p->AudioIndex;
}

Sound *Frame::Sound() const
{
    return p->SoundEffect;
}

uint16_t Frame::Duration() const
{
    return p->Duration;
}

int16_t Frame::ExitFrame() const
{
    return p->ExitFrameID;
}

std::vector<FrameImage *> Frame::Images() const
{
    return p->ImageIndexes;
}

std::vector<Branch *> Frame::Branches() const
{
    return p->Branches;
}

std::vector<Overlay *> Frame::MouthOverlays() const
{
    return p->MouthOverlays;
}

Frame::Frame(FramePrivate *priv)
    :p(priv) {}

Frame::~Frame()
{
#ifdef DEBUG
    cerr << "~F";
#endif
    delete p;
}

uint32_t Image::ImageID() const
{
    return p->ImageID;
}

uint32_t Image::Size() const
{
    return p->ImageDataSize;
}

bool Image::Compressed() const
{
    return p->Compressed;
}

std::vector<uint8_t> Image::Data() const
{
    return p->ImageData;
}

uint16_t Image::Width() const
{
    return p->Width;
}

uint16_t Image::Height() const
{
    return p->Height;
}

bool Image::WriteToFile(std::filesystem::path file)
{
    return p->WriteToFile(file);
}

Image::Image(ImagePrivate *priv)
    :p(priv) {}

Image::~Image()
{
#ifdef DEBUG
    cerr << "~I" <<p->ImageID;
#endif
    delete p;
}

Image *FrameImage::GetImage() const
{
    return ImagePtr;
}

uint32_t FrameImage::GetImageID() const
{
    if(ImagePtr)
        return ImagePtr->ImageID();

    return 999999999;
}

int16_t FrameImage::OffsetX() const
{
    return _OffsetX;
}

int16_t FrameImage::OffsetY() const
{
    return _OffsetY;
}

Overlay::Type Overlay::OverlayType() const
{
    return p->OverlayType;
}

int16_t Overlay::OffsetX() const
{
    return p->OffsetX;
}

int16_t Overlay::OffsetY() const
{
    return p->OffsetY;
}

uint16_t Overlay::Width() const
{
    return p->Width;
}

uint16_t Overlay::Height() const
{
    return p->Height;
}

Image *Overlay::Image() const
{
    if(!p->Image)
        return nullptr;

    return p->Image;
}

Overlay::Overlay(OverlayPrivate *priv)
    :p(priv) { }

Overlay::~Overlay()
{
#ifdef DEBUG
    cerr << "~O";
#endif

    delete p;
}

uint16_t Branch::FrameID() const
{
    return _FrameID;
}

uint16_t Branch::Probability() const
{
    return _Probability;
}

Sound::Sound(SoundPrivate *priv)
    :p(priv) {}

Sound::~Sound()
{
    delete p;
}

uint32_t Sound::SoundID() const
{
    return p->SoundID;
}

uint32_t Sound::Size() const
{
    return p->RIFFData.size();
}

std::vector<uint8_t> Sound::Data() const
{
    return p->RIFFData;
}


