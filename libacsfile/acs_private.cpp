// Authored in 2025 by ~cat - SOSUMI BONZIBROS
// The function CharacterPrivate::DecodeData originates from Double Agent, GPLv3
// The code outside of the aformentioned function is Public Domain

#include "acs_private.h"
#include "acsfile.h"

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <stdlib.h>

#define FLAG_VOICE_OUTPUT        (1u << 4)
#define FLAG_BALLOON_DISABLED    (1u << 8)
#define FLAG_BALLOON_ENABLED     (1u << 9)

// Word balloon styles (bits 16–18)
#define FLAG_BALLOON_STYLE_MASK   (0x7u << 16)
#define FLAG_BALLOON_SIZE_TO_TEXT (1u << 16)
#define FLAG_BALLOON_AUTOHIDE_OFF (1u << 17)
#define FLAG_BALLOON_AUTOPACE_OFF (1u << 18)

// Other flags
#define FLAG_STANDARD_ANIMATIONS  (1u << 20)

using namespace libacsfile;
using namespace std;

CharacterPrivate::CharacterPrivate(const string& filename)
{
    bool error = false;
    ifstream ifs(filename, ios::binary);
    if (!ifs.is_open())
        throw runtime_error("Cannot open file: " + filename);

    // Load ACS header
    uint16_t tempSig16;
    ifs.read(reinterpret_cast<char*>(&tempSig16), sizeof(uint16_t));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read ACS signature");
    }
    ifs.seekg(0, ios::beg);

    uint32_t tempSig;
    ifs.read(reinterpret_cast<char*>(&tempSig), sizeof(uint32_t));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read ACS signature");
    }

    if(tempSig16 == UTOPIA_LE_MAGIC)
    {
        Type = Character::UtopiaLE;
        LoadUtopiaLECharacter(ifs);
    }
    if(tempSig == AGENT_CHAR_150_MAGIC)
    {
        Type = Character::Agent15;
        LoadACS15Character(ifs);
    }
    if(tempSig == AGENT_CHAR_20_MAGIC)
    {
        Type = Character::Agent20;
        LoadACS2Character(ifs);
    }

    if(Type == Character::Invalid)
    {
        ifs.close();
        throw runtime_error("Invalid ACS file signature");
    }

    ifs.close();
}

CharacterPrivate::~CharacterPrivate()
{
    for(auto&[k, ptr] : animations) {
        delete ptr;
    }
    for(auto&[k, ptr] : images) {
        delete ptr;
    }
}

void CharacterPrivate::LoadUtopiaLECharacter(std::ifstream &ifs)
{
    throw runtime_error("Utopia (BOB, Little Endian) character support unimplemented.");
}

void CharacterPrivate::LoadACS15Character(std::ifstream &ifs)
{
    uint32_t tempSig;
    ifs.read(reinterpret_cast<char*>(&tempSig), sizeof(uint32_t));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read COM structured storage signature");
    }

    if(tempSig == AGENT_CHAR_151_MAGIC)
    {
        // Handle COM Structured Storage document
    }

    throw runtime_error("Agent 1.5 character support unimplemented.");
}

void CharacterPrivate::LoadACS2Character(std::ifstream &ifs)
{
    ifs.read(reinterpret_cast<char*>(&ACS2CharacterInfo), sizeof(ACSLOCATOR));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read ACS CharacterInfo Offset");
    }
    ifs.read(reinterpret_cast<char*>(&ACS2AnimationInfo), sizeof(ACSLOCATOR));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read ACS AnimationList Offset");
    }
    ifs.read(reinterpret_cast<char*>(&ACS2ImageInfo), sizeof(ACSLOCATOR));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read ACS ImageList Offset");
    }
    ifs.read(reinterpret_cast<char*>(&ACS2AudioInfo), sizeof(ACSLOCATOR));
    if(ifs.fail())
    {
        ifs.close();
        throw runtime_error("Failed to read ACS AudioList Offset");
    }

    if(!LoadCharacterData(ifs))
    {
        ifs.close();
        throw runtime_error("Failed to read ACS character metadata");
    }

    // We process image & audio data first before animations
    // so that we may link pointers to the animation data
    if(!LoadImageData(ifs))
    {
        ifs.close();
        throw runtime_error("Failed to read ACS images");
    }

    if(!LoadSoundData(ifs))
    {
        ifs.close();
        throw runtime_error("Failed to read ACS sounds");
    }

    if(!LoadAnimationData(ifs))
    {
        ifs.close();
        throw runtime_error("Failed to read ACS animations");
    }

    // TODO: add pointers to states
    acsValid = true;
}

uint32_t CharacterPrivate::DecodeData(const vector<uint8_t>& src, vector<uint8_t>& trg, uint32_t offset = 0)
{
    // Implementation from Double Agent
    if (src.size() <= 7 || src[0] != 0)
        return 0;

    const uint8_t* lSrcPtr = src.data();
    const uint8_t* lSrcEnd = src.data() + src.size();
    uint8_t* lTrgPtr = trg.data()+offset;
    uint8_t* lTrgEnd = trg.data() + trg.size();

    uint32_t lSrcQuad = 0;
    uint8_t  lTrgByte = 0;
    uint32_t lBitCount = 0;
    uint32_t lSrcOffset = 0;
    uint32_t lRunLgth = 0;
    uint32_t lRunCount = 0;

    // Check trailing padding
    for (lBitCount = 1; (*(lSrcEnd - lBitCount) == 0xFF); lBitCount++)
        if (lBitCount > 6) break;

    if (lBitCount < 6) return 0;

    lBitCount = 0;
    lSrcPtr += 5;

    while (lSrcPtr < lSrcEnd && lTrgPtr < lTrgEnd)
    {
        memcpy(&lSrcQuad, lSrcPtr - sizeof(uint32_t), sizeof(uint32_t));

        if (lSrcQuad & (1u << (lBitCount & 0xFFFF)))
        {
            lSrcOffset = 1;

            if (lSrcQuad & (1u << ((lBitCount + 1) & 0xFFFF)))
            {
                if (lSrcQuad & (1u << ((lBitCount + 2) & 0xFFFF)))
                {
                    if (lSrcQuad & (1u << ((lBitCount + 3) & 0xFFFF)))
                    {
                        lSrcQuad >>= (lBitCount + 4) & 0xFFFF;
                        lSrcQuad &= 0x000FFFFF;
                        if (lSrcQuad == 0x000FFFFF) break;
                        lSrcQuad += 4673;
                        lBitCount += 24;
                        lSrcOffset = 2;
                    }
                    else
                    {
                        lSrcQuad >>= (lBitCount + 4) & 0xFFFF;
                        lSrcQuad &= 0x00000FFF;
                        lSrcQuad += 577;
                        lBitCount += 16;
                    }
                }
                else
                {
                    lSrcQuad >>= (lBitCount + 3) & 0xFFFF;
                    lSrcQuad &= 0x000001FF;
                    lSrcQuad += 65;
                    lBitCount += 12;
                }
            }
            else
            {
                lSrcQuad >>= (lBitCount + 2) & 0xFFFF;
                lSrcQuad &= 0x0000003F;
                lSrcQuad += 1;
                lBitCount += 8;
            }

            lSrcPtr += (lBitCount / 8);
            lBitCount &= 7;

            memcpy(&lRunLgth, lSrcPtr - sizeof(uint32_t), sizeof(uint32_t));
            lRunCount = 0;
            while (lRunLgth & (1u << ((lBitCount + lRunCount) & 0xFFFF)))
            {
                lRunCount++;
                if (lRunCount > 11) break;
            }

            lRunLgth >>= (lBitCount + lRunCount + 1) & 0xFFFF;
            lRunLgth &= (1u << lRunCount) - 1;
            lRunLgth += 1u << lRunCount;
            lRunLgth += lSrcOffset;
            lBitCount += lRunCount * 2 + 1;

            if (lTrgPtr + lRunLgth > lTrgEnd) break;
            if (lTrgPtr < trg.data() + lSrcQuad) break;

            while (static_cast<long>(lRunLgth) > 0)
            {
                lTrgByte = *(lTrgPtr - lSrcQuad);
                *(lTrgPtr++) = lTrgByte;
                lRunLgth--;
            }
        }
        else
        {
            lSrcQuad >>= (lBitCount + 1) & 0xFFFF;
            lBitCount += 9;

            lTrgByte = static_cast<uint8_t>(lSrcQuad & 0xFF);
            *(lTrgPtr++) = lTrgByte;
        }

        lSrcPtr += lBitCount / 8;
        lBitCount &= 7;
    }

    return static_cast<uint32_t>(lTrgPtr - trg.data());
}

bool CharacterPrivate::LoadCharacterData(ifstream &ifs)
{
    // Load ACSCHARACTERINFO
    ACSLOCATOR localizedInfoLocator{};
    ifs.seekg(ACS2CharacterInfo.Offset, ios::beg);
    if (ifs.fail()) return false;
    // Reading ACSCHARACTERINFO fields
    if (!ifs.read(reinterpret_cast<char*>(&MinorVersion), sizeof(uint16_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&MajorVersion), sizeof(uint16_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&localizedInfoLocator), sizeof(ACSLOCATOR))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&CharacterID), sizeof(GUID))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&CharacterWidth), sizeof(uint16_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&CharacterHeight), sizeof(uint16_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&TransparentColorIndex), sizeof(uint8_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&Flags), sizeof(uint32_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&AnimationSetMajorVersion), sizeof(uint16_t))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&AnimationSetMinorVersion), sizeof(uint16_t))) return false;

    // Reading VOICEINFO fields
    // some characters (like the o2k assistants) do not have these fields
    if(Flags & CHAR_STYLE_TTS)
    {
        if (!ifs.read(reinterpret_cast<char*>(&EngineID), sizeof(GUID))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&ModeID), sizeof(GUID))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&Speed), sizeof(uint32_t))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&Pitch), sizeof(uint16_t))) return false;
        bool hasExtraData = false;
        if (!ifs.read(reinterpret_cast<char*>(&hasExtraData), sizeof(bool))) return false;
        if (hasExtraData) {
            if (!ifs.read(reinterpret_cast<char*>(&LangID), sizeof(uint16_t))) return false;
            Dialect = ReadString(ifs);
            if (!ifs.read(reinterpret_cast<char*>(&Gender), sizeof(uint16_t))) return false;
            if (!ifs.read(reinterpret_cast<char*>(&Age), sizeof(uint16_t))) return false;
            Style = ReadString(ifs);
        }
    }

    // Reading BALOONINFO fields
    bool hasBaloonInfo = true;
    if(hasBaloonInfo)
    {
        if (!ifs.read(reinterpret_cast<char*>(&TextLines), sizeof(uint8_t))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&CharsPerLine), sizeof(uint8_t))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&ForegroundColor), sizeof(RGBQUAD))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&BackgroundColor), sizeof(RGBQUAD))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&BorderColor), sizeof(RGBQUAD))) return false;
        FontName = ReadString(ifs);
        if (!ifs.read(reinterpret_cast<char*>(&FontHeight), sizeof(int32_t))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&FontWeight), sizeof(int32_t))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&Italicized), sizeof(bool))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&UnknownBalloonFlag), sizeof(uint8_t))) return false;
    }

    // Resuming ACSCHARACTERINFO fields
    uint32_t paletteCount{};
    if (!ifs.read(reinterpret_cast<char*>(&paletteCount), sizeof(paletteCount))) return false;
    Palette.resize(paletteCount);
    if (paletteCount > 0)
        if (!ifs.read(reinterpret_cast<char*>(Palette.data()), paletteCount * sizeof(RGBQUAD))) return false;

    if (!ifs.read(reinterpret_cast<char*>(&TrayIconEnabled), sizeof(bool))) return false;
    if(TrayIconEnabled)
    {
        // Reading TRAYICON fields
        if (!ifs.read(reinterpret_cast<char*>(&MonoSize), sizeof(uint32_t))) return false;
        if(MonoSize > 0)
        {
            // TODO: handle this rn we just seek over
            ifs.seekg(MonoSize, ios::cur);
        }
        if (!ifs.read(reinterpret_cast<char*>(&ColorSize), sizeof(uint32_t))) return false;
        if(ColorSize > 0)
        {
            // TODO: handle this rn we just seek over
            ifs.seekg(ColorSize, ios::cur);
        }
    }

    // Resuming ACSCHARACTERINFO fields
    uint16_t stateCount{};
    if (!ifs.read(reinterpret_cast<char*>(&stateCount), sizeof(stateCount))) return false;
    for (uint16_t i = 0; i < stateCount; i++) {
        string stateName = ReadString(ifs);
        vector<string> stateAnimations;
        uint16_t animationCount{};
        if (!ifs.read(reinterpret_cast<char*>(&animationCount),
                      sizeof(animationCount))) return false;
        stateAnimations.resize(animationCount);
        for (uint16_t i = 0; i < animationCount; i++) {
            string animationName = ReadString(ifs);
            stateAnimations.push_back(animationName);
        }
        States[stateName] = stateAnimations;
    }

    // load the ACSLOCALIZEDINFO data
    uint16_t localizationCount{};
    ifs.seekg(localizedInfoLocator.Offset, ios::beg);
    if (ifs.fail()) return false;
    if (!ifs.read(reinterpret_cast<char*>(&localizationCount), sizeof(uint16_t))) return false;

    if(localizationCount > 0)
    {
        for(int i = 0; i < localizationCount; ++i)
        {
            uint16_t localeID;
            if (!ifs.read(reinterpret_cast<char*>(&localeID), sizeof(uint16_t))) return false;
            if(localeID == 9)
            {
                // TODO: handle all other localizations
                CharacterName.assign(ReadString(ifs));
                CharacterDescription.assign(ReadString(ifs));
                CharacterExtraData.assign(ReadString(ifs));
            }
            else
            {
                for(int j = 0; j < 3; ++j)
                    SkipString(ifs);
            }
        }
    }

    return true;
}

bool CharacterPrivate::LoadAnimationData(ifstream &ifs)
{
    uint32_t listcount = 0;
    ifs.seekg(ACS2AnimationInfo.Offset, ios::beg);
    if (ifs.fail()) return false;

    if (!ifs.read(reinterpret_cast<char*>(&listcount), sizeof(uint32_t))) return false;
    if (ifs.fail()) return false;

    if(listcount > 0)
    {
        map<string, ACSLOCATOR> animationMap;
        for(int i = 0; i < listcount; i++)
        {
            string animationName = ReadString(ifs);
            ACSLOCATOR locator{};
            if (!ifs.read(reinterpret_cast<char*>(&locator), sizeof(ACSLOCATOR))) return false;
            animationMap[animationName] = locator;
        }

        for(map<string, ACSLOCATOR>::iterator it = animationMap.begin();
             it != animationMap.end();
             ++it)
        {
            AnimationPrivate *animationInfo = new AnimationPrivate(ifs, it->second.Offset, this);
            animationInfo->DisplayName = it->first;
            Animation *publicAnimation = new Animation(animationInfo);
            animations[animationInfo->Name] = publicAnimation;
        }
    }

    return true;
}

bool CharacterPrivate::LoadImageData(ifstream &ifs)
{
    uint32_t listcount = 0;
    ifs.seekg(ACS2ImageInfo.Offset, ios::beg);
    if (ifs.fail()) return false;

    if (!ifs.read(reinterpret_cast<char*>(&listcount), sizeof(uint32_t))) return false;
    if (ifs.fail()) return false;

    if(listcount > 0)
    {
        map<uint16_t, ACSLOCATOR> imageMap;
        for(int i = 0; i < listcount; ++i)
        {
            ACSLOCATOR locator{};
            uint32_t checksum{};
            if (!ifs.read(reinterpret_cast<char*>(&locator), sizeof(ACSLOCATOR))) return false;
            if (!ifs.read(reinterpret_cast<char*>(&checksum), sizeof(uint32_t))) return false;
            imageMap[i] = locator;
        }

        for(map<uint16_t, ACSLOCATOR>::iterator it = imageMap.begin();
             it != imageMap.end();
             ++it)
        {
            ImagePrivate *imageInfo = new ImagePrivate(ifs, it->second.Offset, this);
            imageInfo->ImageID = it->first;
            Image *publicImage = new Image(imageInfo);
            imageInfo->PublicImage = publicImage;
            images[it->first] = publicImage;
        }
    }
    return true;
}

bool CharacterPrivate::LoadSoundData(std::ifstream &ifs)
{
    uint32_t listcount = 0;
    ifs.seekg(ACS2AudioInfo.Offset, ios::beg);
    if (ifs.fail()) return false;

    if (!ifs.read(reinterpret_cast<char*>(&listcount), sizeof(uint32_t))) return false;
    if (ifs.fail()) return false;

    if(listcount > 0)
    {
        map<uint16_t, ACSLOCATOR> soundMap;
        for(int i = 0; i < listcount; ++i)
        {
            ACSLOCATOR locator{};
            uint32_t checksum{};
            if (!ifs.read(reinterpret_cast<char*>(&locator), sizeof(ACSLOCATOR))) return false;
            if (!ifs.read(reinterpret_cast<char*>(&checksum), sizeof(uint32_t))) return false;
            soundMap[i] = locator;
        }

        for(map<uint16_t, ACSLOCATOR>::iterator it = soundMap.begin();
             it != soundMap.end();
             ++it)
        {
            SoundPrivate *soundInfo = new SoundPrivate(ifs, it->second.Offset, it->second.Size, it->first);
            if(soundInfo != nullptr)
            {
                Sound *sound = new Sound(soundInfo);
                sounds[it->first] = sound;
            }
        }
    }

    return true;
}

string CharacterPrivate::ReadString(ifstream &ifs)
{
    uint32_t length;
    string result;
    result.clear();
    if (!ifs.read(reinterpret_cast<char*>(&length), sizeof(uint32_t))) return result;
    if (length == 0)
        return result;

    vector<uint16_t> buf(length + 1); // include terminator
    if (!ifs.read(reinterpret_cast<char*>(buf.data()), buf.size() * sizeof(uint16_t))) return result;
    result.reserve(buf.size());

    for (uint16_t wc : buf) {
        if (wc == L'\0') break; // stop at null terminator
        result.push_back(static_cast<char>(wc & 0xFF));
    }

    return result;
}

Image *CharacterPrivate::FindImageByID(uint16_t ImageID)
{
    if(images.find(ImageID) != images.end())
        return images[ImageID];

    return nullptr;
}

Sound *CharacterPrivate::FindSoundByID(uint16_t SoundID)
{
    if(SoundID == 65535)
        return nullptr;

    if(sounds.find(SoundID) != sounds.end())
        return sounds[SoundID];

    return nullptr;
}

std::vector<RGBQUAD> CharacterPrivate::BitmapPalette() const
{
    return Palette;
}

void CharacterPrivate::SkipString(ifstream &ifs)
{
    uint32_t length;
    if (!ifs.read(reinterpret_cast<char*>(&length), sizeof(uint32_t))) return;
    if (length == 0)
        return;

    vector<uint16_t> buf(length + 1); // include terminator
    if (!ifs.read(reinterpret_cast<char*>(buf.data()), buf.size() * sizeof(uint16_t))) return;

    return;
}

string CharacterPrivate::GuidToString(GUID guid)
{
    char guid_cstr[39];
    snprintf(guid_cstr, sizeof(guid_cstr),
             "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
             guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
             guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    return string(guid_cstr);
}

AnimationPrivate::AnimationPrivate(ifstream &ifs, uint32_t offset, CharacterPrivate *priv)
    :c(priv)
{
    //  ACSANIMATIONINFO type
    ifs.seekg(offset, ios::beg);
    if (ifs.fail()) return;
    Name = CharacterPrivate::ReadString(ifs);
    if (!ifs.read(reinterpret_cast<char*>(&Transition), sizeof(uint8_t))) return;
    ReturnAnimation = CharacterPrivate::ReadString(ifs);

    uint16_t frameCount{};
    if (!ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(uint16_t))) return;
    for(int i = 0; i < frameCount; ++i)
    {
        auto frame = new FramePrivate(ifs, c);
        auto publicFrame = new Frame(frame);
        Frames[i] = publicFrame;
    }
}

AnimationPrivate::~AnimationPrivate()
{
    for(auto &[k,ptr] : Frames)
        delete ptr;
}

ImagePrivate::ImagePrivate(ifstream &ifs, uint32_t offset, CharacterPrivate *priv)
    :c(priv)
{
    ifs.seekg(offset, ios::beg);
    if (ifs.fail()) return;
    if (!ifs.read(reinterpret_cast<char*>(&Unknown), sizeof(uint8_t))) return;
    if (!ifs.read(reinterpret_cast<char*>(&Width), sizeof(uint16_t))) return;
    if (!ifs.read(reinterpret_cast<char*>(&Height), sizeof(uint16_t))) return;
    if (!ifs.read(reinterpret_cast<char*>(&Compressed), sizeof(bool))) return;

    // read the image data
    if (!ifs.read(reinterpret_cast<char*>(&ImageDataSize), sizeof(uint32_t))) return;
    if(ImageDataSize > 0)
    {
        if(Compressed > 0)
        {
            vector<uint8_t> ImageDataCompressed(ImageDataSize);
            uint16_t uncompressedSize = ((Width + 3) & 0xFC) * Height;
            if (!ifs.read(reinterpret_cast<char*>(ImageDataCompressed.data()), ImageDataCompressed.size()))
                return;

            ImageData.resize(uncompressedSize);
            c->DecodeData(ImageDataCompressed, ImageData);
        }
        else
        {
            ImageData.resize(ImageDataSize);

            if (!ifs.read(reinterpret_cast<char*>(ImageData.data()), ImageDataSize))
                return;
        }
    }
    // Read the region data
    uint32_t regionCompressedSize{};
    uint32_t regionUncompressedSize{};
    if (!ifs.read(reinterpret_cast<char*>(&regionCompressedSize), sizeof(uint32_t))) return;
    if (!ifs.read(reinterpret_cast<char*>(&regionUncompressedSize), sizeof(uint32_t))) return;
    if(regionCompressedSize > 0)
    {
        // TODO: implement
        // region data is compressed
    }
    else
    {
        // TODO: implement
        // uncompressed, data will be a raw RGNDATA structure
    }
}

ImagePrivate::~ImagePrivate()
{
    ImageData.clear();
}

bool ImagePrivate::WriteToFile(std::filesystem::path file)
{
    std::ofstream ofs(file, ios::out);
    if(!ofs)
        goto fail;

    BITMAPFILEHEADER fh;
    fh.bfType = 0x4D42;
    fh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + Width * Height * 3;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;
    fh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*c->BitmapPalette().size();

    ofs.write(reinterpret_cast<char*>(&fh), sizeof(fh));

    BITMAPINFO bi;
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = Width;
    bi.bmiHeader.biHeight = Height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 8;
    bi.bmiHeader.biCompression = 0;
    // 4 byte alignment
    bi.bmiHeader.biSizeImage = (((Width+3)/4)*4)*Height;
    bi.bmiHeader.biXPelsPerMeter = 0;
    bi.bmiHeader.biYPelsPerMeter = 0;
    bi.bmiHeader.biClrUsed = c->BitmapPalette().size();
    bi.bmiHeader.biClrImportant = c->BitmapPalette().size();


    ofs.write(reinterpret_cast<char*>(&bi.bmiHeader), sizeof(BITMAPINFOHEADER));
    for (uint32_t i = 0; i < c->BitmapPalette().size(); i++)
        ofs.write(reinterpret_cast<char*>(&c->BitmapPalette()[i]), sizeof(RGBQUAD));

    ofs.write(reinterpret_cast<char*>(ImageData.data()), ImageData.size());
    ofs.close();
    return true;

fail:
    if(ofs.is_open())
        ofs.close();
    return false;
}

FramePrivate::FramePrivate(ifstream &ifs, CharacterPrivate *priv)
    :c(priv)
{
    uint16_t frameImageCount{};
    if (!ifs.read(reinterpret_cast<char*>(&frameImageCount), sizeof(uint16_t))) return;
    for(int i = 0; i < frameImageCount; ++i)
    {
        auto fr = new FrameImage();
        uint32_t imgID{};
        uint16_t offset_x{};
        uint16_t offset_y{};
        if (!ifs.read(reinterpret_cast<char*>(&imgID), sizeof(uint32_t)))
        {
            delete fr;
            return;
        }
        if (!ifs.read(reinterpret_cast<char*>(&offset_x), sizeof(uint16_t)))
        {
            delete fr;
            return;
        }
        if (!ifs.read(reinterpret_cast<char*>(&offset_y), sizeof(uint16_t)))
        {
            delete fr;
            return;
        }

        fr->_OffsetX = offset_x;
        fr->_OffsetY = offset_y;

        auto imgPtr = c->FindImageByID(imgID);
        if(imgPtr == nullptr)
        {
            delete fr;
            return;
        }

        fr->ImagePtr = imgPtr;
        ImageIndexes.push_back(fr);
    }
    if (!ifs.read(reinterpret_cast<char*>(&AudioIndex), sizeof(uint16_t))) return;
    if (!ifs.read(reinterpret_cast<char*>(&Duration), sizeof(uint16_t))) return;
    if (!ifs.read(reinterpret_cast<char*>(&ExitFrameID), sizeof(int16_t))) return;

    uint8_t branchCount{};
    if (!ifs.read(reinterpret_cast<char*>(&branchCount), sizeof(uint8_t))) return;
    for(int i = 0; i < branchCount; ++i)
    {
        auto branch = new Branch;
        if (!ifs.read(reinterpret_cast<char*>(&branch->_FrameID), sizeof(uint16_t))) return;
        if (!ifs.read(reinterpret_cast<char*>(&branch->_Probability), sizeof(uint16_t))) return;

        Branches.push_back(branch);
    }

    uint8_t overlayCount{};
    if (!ifs.read(reinterpret_cast<char*>(&overlayCount), sizeof(uint8_t))) return;
    for(int i = 0; i < overlayCount; ++i)
    {
        auto overlay = new OverlayPrivate(ifs, c);
        auto overlayPublic = new Overlay(overlay);
        MouthOverlays.push_back(overlayPublic);
    }

    SoundEffect = c->FindSoundByID(AudioIndex);
    return;
}

FramePrivate::~FramePrivate()
{
    for(auto &m : MouthOverlays)
        delete m;
}

OverlayPrivate::OverlayPrivate(ifstream &ifs, CharacterPrivate *priv)
    :c(priv)
{
    if(!ifs.read(reinterpret_cast<char*>(&OverlayType), sizeof(uint8_t))) return;
    if(!ifs.read(reinterpret_cast<char*>(&ReplaceTop), sizeof(bool))) return;
    if(!ifs.read(reinterpret_cast<char*>(&ImageID), sizeof(uint16_t))) return;
    if(!ifs.read(reinterpret_cast<char*>(&Unknown), sizeof(uint8_t))) return;
    if(!ifs.read(reinterpret_cast<char*>(&HasRegionData), sizeof(bool))) return;
    if(!ifs.read(reinterpret_cast<char*>(&OffsetX), sizeof(int16_t))) return;
    if(!ifs.read(reinterpret_cast<char*>(&OffsetY), sizeof(int16_t))) return;
    if(!ifs.read(reinterpret_cast<char*>(&Width), sizeof(uint16_t))) return;
    if(!ifs.read(reinterpret_cast<char*>(&Height), sizeof(uint16_t))) return;
    if(HasRegionData)
    {
        // This region data should not be compressed
        uint32_t dataSize{};
        if(!ifs.read(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t))) return;
        RGNDATAHEADER regionHeader{};
        if(!ifs.read(reinterpret_cast<char*>(&regionHeader), sizeof(RGNDATAHEADER))) return;
        if(regionHeader.nCount > 0)
        {
            // TODO: finish impl, find an agent that uses this???
        }
    }
}

SoundPrivate::SoundPrivate(std::ifstream &ifs, uint32_t offset, uint32_t size, uint32_t id)
    :SoundID(id)
{
    ifs.seekg(offset, ios::beg);
    if(ifs.fail())
        return;

    RIFFData.resize(size);
    if(!ifs.read(reinterpret_cast<char*>(RIFFData.data()), size))
        return;
}

SoundPrivate::~SoundPrivate()
{
    RIFFData.clear();
}

bool SoundPrivate::WriteToFile(std::filesystem::path &file)
{
    std::ofstream ofs(file, ios::out);
    if(!ofs)
        goto fail;

    ofs.write(reinterpret_cast<char*>(RIFFData.data()), RIFFData.size());
    ofs.close();
    return true;

fail:
    if(ofs.is_open())
        ofs.close();
    return false;
}
