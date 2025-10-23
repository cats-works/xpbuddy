// libacsfile - Authored in 2025 by ~cat - SOSUMI BONZIBROS
// The code is Public Domain

#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>

#ifndef _WIN32
#include <acs_wintypes.h>
#else
#include <windows.h>
#endif

namespace libacsfile {
    class ImagePrivate;
    class OverlayPrivate;
    class FramePrivate;
    class AnimationPrivate;
    class CharacterPrivate;
    class SoundPrivate;

    class Sound {
    public:
        uint32_t SoundID() const;
        uint32_t Size() const;
        std::vector<uint8_t> Data() const;
        bool WriteToFile(std::filesystem::path file);
    private:
        friend class libacsfile::CharacterPrivate;
        explicit Sound(libacsfile::SoundPrivate *priv);
        ~Sound();
        libacsfile::SoundPrivate *p = nullptr;
    };

    class Image {
    public:
        uint32_t ImageID() const;
        uint32_t Size() const;
        bool Compressed() const;
        std::vector<uint8_t> Data() const;
        uint16_t Width() const;
        uint16_t Height() const;
        bool WriteToFile(std::filesystem::path file);
    private:
        friend class libacsfile::CharacterPrivate;
        explicit Image(libacsfile::ImagePrivate *priv);
        ~Image();
        libacsfile::ImagePrivate *p = nullptr;
    };    

    class Overlay {
    public:
        enum Type {
            Closed = 0x00,
            WideOpen1 = 0x01,
            WideOpen2 = 0x02,
            WideOpen3 = 0x03,
            WideOpen4 = 0x04,
            Medium = 0x05,
            Narrow = 0x06
        };
        Type OverlayType() const;
        int16_t OffsetX() const;
        int16_t OffsetY() const;
        uint16_t Width() const;
        uint16_t Height() const;
        libacsfile::Image* Image() const;
    private:
        friend class libacsfile::OverlayPrivate;
        friend class libacsfile::FramePrivate;
        explicit Overlay(libacsfile::OverlayPrivate *priv);
        ~Overlay();
        libacsfile::OverlayPrivate *p = nullptr;
    };

    class Frame;
    class Branch {
    public:
        uint16_t FrameID() const;
        uint16_t Probability() const;
    private:
        friend class libacsfile::FramePrivate;
        explicit Branch() = default;
        uint16_t _FrameID{};
        uint16_t _Probability{};
    };

    class FrameImage {
    public:
        libacsfile::Image* GetImage() const;
        uint32_t GetImageID() const;
        int16_t OffsetX() const;
        int16_t OffsetY() const;
    private:
        friend class libacsfile::FramePrivate;
        explicit FrameImage() = default;
        libacsfile::Image *ImagePtr = nullptr;
        int16_t _OffsetX{};
        int16_t _OffsetY{};
    };

    class Frame {
    public:
        uint16_t AudioIndex() const;
        libacsfile::Sound* Sound() const;
        uint16_t Duration() const;
        int16_t ExitFrame() const;
        std::vector<FrameImage*> Images() const;
        std::vector<Branch*> Branches() const;
        std::vector<Overlay*> MouthOverlays() const;
    private:
        friend class libacsfile::AnimationPrivate;
        explicit Frame(libacsfile::FramePrivate *priv);
        ~Frame();
        libacsfile::FramePrivate *p = nullptr;
    };

    class Animation {
    public:
        enum TransitionType {
            TransitionReturnAnimation = 0x00,
            TransitionExitBranches = 0x01,
            TransitionNone = 0x02
        };
        std::string Name() const;
        TransitionType Transition() const;
        std::string ReturnAnimation() const;
        std::map<uint16_t, Frame*> Frames() const;
    private:
        friend class libacsfile::CharacterPrivate;
        explicit Animation(libacsfile::AnimationPrivate *priv);
        ~Animation();
        libacsfile::AnimationPrivate *p = nullptr;
    };

    class Character {
    public:
        enum Type {
            Invalid,
            UtopiaLE,
            UtopiaBE,
            Agent15,
            Agent20
        };
        Character() = default;
        ~Character();
        bool Load(const std::string &filename);
        bool Loaded();
        std::string GUID() const;
        std::string Name() const;
        std::string Description() const;
        uint16_t Width() const;
        uint16_t Height() const;
        bool TTSEnabled() const;
        std::string TTSEngineGUID() const;
        std::string TTSModeGUID() const;
        uint32_t VoiceSpeed() const;
        uint16_t VoicePitch() const;
        uint16_t Gender() const;
        uint16_t Age() const;
        std::string Style() const;
        RGBQUAD TransparentColor() const;
        std::vector<RGBQUAD> ColorPalette() const;
        bool BalloonEnabled() const;
        std::string BalloonFont() const;
        bool TrayIconEnabled() const;

        std::map<std::string, std::vector<std::string>> States() const;

        std::vector<std::string> AnimationNames() const;
        std::map<std::string, Animation*> Animations() const;
        libacsfile::Animation* GetAnimation(std::string &name);
        bool HasAnimation(const std::string &name);
        bool HasState(const std::string &state);

        std::map<uint16_t, Image*> Images() const;
        std::map<uint16_t, Sound*> Sounds() const;
    private:
        libacsfile::CharacterPrivate *p = nullptr;
    };
}
