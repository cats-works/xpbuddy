// libacsfile - Authored in 2025 by ~cat - SOSUMI BONZIBROS
// The code is Public Domain

#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <filesystem>

#define UTOPIA_BE_MAGIC         0x4C50
#define UTOPIA_LE_MAGIC         0x504C
#define AGENT_CHAR_15_MAGIC     0xABCDABC1
#define AGENT_CHAR_150_MAGIC    0xE011CFD0
#define AGENT_CHAR_151_MAGIC    0xE11AB1A1
#define AGENT_CHAR_20_MAGIC     0xABCDABC3

#define CHAR_STYLE_TTS          0x00000020
#define CHAR_STYLE_BALLOON      0x00000200
#define CHAR_STYLE_SIZE_TO_TEXT 0x00000020
#define CHAR_STYLE_NO_AUTO_HIDE 0x00020000
#define CHAR_STYLE_NO_AUTO_PACE 0x00040000
#define CHAR_STYLE_STANDARD     0x00100000

#ifdef WIN32
#include <windows.h>
#else
#include "acs_wintypes.h"
#endif // WIN32


#include "acsfile.h"

//#pragma pack(push, 1)
struct ACSLOCATOR {
    uint32_t Offset;
    uint32_t Size;
};
//#pragma pack(pop)

struct ICONIMAGE {
    BITMAPINFOHEADER Header;
    std::vector<RGBQUAD> ColorTable;
    std::vector<uint8_t> XORBits;
    std::vector<uint8_t> ANDBits;
    bool read(std::ifstream &ifs) {
        if (!ifs.read(reinterpret_cast<char*>(&Header), sizeof(BITMAPINFOHEADER))) return false;
        uint16_t xorBitCount = 0, andBitCount = 0;
        if (!ifs.read(reinterpret_cast<char*>(&xorBitCount), sizeof(uint16_t))) return false;
        if(xorBitCount > 0)
        {
            for(int i = 0; i < xorBitCount; i++)
            {
                uint8_t bit = 0;
                if (!ifs.read(reinterpret_cast<char*>(&bit), sizeof(uint8_t))) return false;
            }
        }
        return true;
    }
};



namespace libacsfile {

    class SoundPrivate {
    private:
        friend class Sound;
        friend class CharacterPrivate;
        explicit SoundPrivate(std::ifstream &ifs, uint32_t offset, uint32_t size, uint32_t id);
        ~SoundPrivate();
        bool WriteToFile(std::filesystem::path &file);
        uint32_t SoundID{};
        std::vector<uint8_t> RIFFData;
    };

    class CharacterPrivate;
    class ImagePrivate
    {
    private:
        friend class libacsfile::Image;
        friend class libacsfile::CharacterPrivate;
        explicit ImagePrivate(std::ifstream &ifs, uint32_t offset, CharacterPrivate *priv);
        ~ImagePrivate();
        bool WriteToFile(std::filesystem::path file);
        uint32_t ImageID{};
        uint8_t Unknown{};
        uint16_t Width{};
        uint16_t Height{};
        bool Compressed{};
        std::vector<uint8_t> ImageData;
        uint32_t ImageDataSize;
        BITMAPINFO *bi;
        libacsfile::Image *PublicImage = nullptr;
        libacsfile::CharacterPrivate *c = nullptr;
    };

    class OverlayPrivate
    {
    private:
        friend class libacsfile::Overlay;
        friend class libacsfile::FramePrivate;
        explicit OverlayPrivate(std::ifstream &ifs, CharacterPrivate *priv);
        Overlay::Type OverlayType{};
        bool ReplaceTop{};
        uint16_t ImageID{};
        libacsfile::Image* Image = nullptr;
        uint8_t Unknown{};
        bool HasRegionData{};
        int16_t OffsetX{};
        int16_t OffsetY{};
        uint16_t Width{};
        uint16_t Height{};
        libacsfile::CharacterPrivate *c = nullptr;
    };

    class FramePrivate
    {
    private:
        friend class Frame;
        friend class AnimationPrivate;
        explicit FramePrivate(std::ifstream &ifs, CharacterPrivate *priv);
        ~FramePrivate();
        std::vector<FrameImage*> ImageIndexes{};
        Sound* SoundEffect = nullptr;
        uint16_t AudioIndex{};
        uint16_t Duration{};
        int16_t ExitFrameID{};
        std::vector<Branch*> Branches{};
        std::vector<Overlay*> MouthOverlays{};
        libacsfile::CharacterPrivate *c = nullptr;
    };

    class AnimationPrivate
    {
    private:
        friend class libacsfile::Animation;
        friend class libacsfile::CharacterPrivate;
        explicit AnimationPrivate(std::ifstream &ifs, uint32_t offset, CharacterPrivate *priv);
        ~AnimationPrivate();
        std::string Name{};
        std::string DisplayName{};
        libacsfile::Animation::TransitionType Transition{};
        std::string ReturnAnimation{};
        std::map<uint16_t, Frame*> Frames{};
        libacsfile::CharacterPrivate *c = nullptr;
    };

    class CharacterPrivate
    {
    public:
        static std::string ReadString(std::ifstream &ifs);
        Image* FindImageByID(uint16_t ImageID);
        Sound* FindSoundByID(uint16_t SoundID);
        std::vector<RGBQUAD> BitmapPalette() const;
        uint32_t DecodeData(const std::vector<uint8_t> &src, std::vector<uint8_t> &trg, uint32_t offset);
    private:
        friend class Character;
        CharacterPrivate(const std::string& filename);
        ~CharacterPrivate();
        std::string GuidToString(GUID guid);
        void LoadUtopiaLECharacter(std::ifstream &ifs);
        void LoadACS15Character(std::ifstream &ifs);
        void LoadACS2Character(std::ifstream &ifs);
        bool LoadCharacterData(std::ifstream &ifs);
        bool LoadAnimationData(std::ifstream &ifs);
        bool LoadImageData(std::ifstream &ifs);
        bool LoadSoundData(std::ifstream &ifs);
        void SkipString(std::ifstream &ifs);
    private:
        bool acsValid;
        GUID CharacterID{};
        GUID EngineID{};
        GUID ModeID{};
        ACSLOCATOR ACS2CharacterInfo;
        ACSLOCATOR ACS2AnimationInfo;
        ACSLOCATOR ACS2ImageInfo;
        ACSLOCATOR ACS2AudioInfo;
        libacsfile::Character::Type Type = libacsfile::Character::Invalid;
        RGBQUAD ForegroundColor{};
        RGBQUAD BackgroundColor{};
        RGBQUAD BorderColor{};

        std::string Dialect;
        std::string Style;
        std::string FontName;

        uint32_t Flags{};
        uint32_t Speed{};
        int32_t FontHeight;
        int32_t FontWeight;

        uint16_t MinorVersion{};
        uint16_t MajorVersion{};
        uint16_t CharacterWidth{};
        uint16_t CharacterHeight{};
        uint16_t AnimationSetMajorVersion{};
        uint16_t AnimationSetMinorVersion{};
        uint16_t Pitch{};
        uint16_t VoiceLangID{};
        uint16_t Gender{};
        uint16_t Age{};
        uint8_t TextLines;
        uint8_t CharsPerLine;
        bool Italicized = false;
        uint8_t UnknownBalloonFlag;
        uint8_t TransparentColorIndex{};

        // Resuming corresponding fields to ACSCHARACTERINFO
        std::vector<RGBQUAD> Palette;
        bool TrayIconEnabled = false;
        uint32_t MonoSize{};
        ICONIMAGE MonoBitmap{};
        uint32_t ColorSize{};
        ICONIMAGE ColorBitmap{};
        std::map<std::string, std::vector<std::string>> States;
        std::map<std::string, std::vector<libacsfile::Animation*>> StatePtrs;

        // Fields correspond with ACSLOCALIZEDINFO
        uint16_t LangID{};
        std::string CharacterName{};
        std::string CharacterDescription{};
        std::string CharacterExtraData{};

        std::map<std::string, libacsfile::Animation*> animations;
        std::map<uint16_t, libacsfile::Image*> images;
        std::map<uint16_t, libacsfile::Sound*> sounds;
    };
}
