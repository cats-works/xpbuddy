// libacsfile - Authored in 2025 by ~cat - SOSUMI BONZIBROS
// The code is Public Domain

#pragma once
#include <cstdint>

#pragma pack(push, 1)
// re-define some required structures
typedef struct _GUID {
    uint32_t  Data1;
    uint16_t Data2;
    uint16_t Data3;
    unsigned char Data4[8];
} GUID;

typedef struct _BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER {
    uint32_t biSize;             // Size of this structure in bytes
    int32_t  biWidth;            // Width of bitmap in pixels
    int32_t  biHeight;           // Height of bitmap in pixels
    uint16_t biPlanes;           // Number of planes (must be 1)
    uint16_t biBitCount;         // Bits per pixel
    uint32_t biCompression;      // Compression type
    uint32_t biSizeImage;        // Size of image data in bytes
    int32_t  biXPelsPerMeter;    // Horizontal resolution (pixels per meter)
    int32_t  biYPelsPerMeter;    // Vertical resolution (pixels per meter)
    uint32_t biClrUsed;          // Number of colors used in the bitmap
    uint32_t biClrImportant;     // Number of important colors
} BITMAPINFOHEADER;

typedef struct _RGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;            // Always 0x00
} RGBQUAD;

typedef struct _BITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO;

typedef struct _RECT {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} RECT;

typedef struct _RGNDATAHEADER {
    uint32_t dwSize;
    uint32_t iType;
    uint32_t nCount;
    uint32_t nRgnSize;
    RECT  rcBound;
} RGNDATAHEADER;

#pragma pack(pop)
