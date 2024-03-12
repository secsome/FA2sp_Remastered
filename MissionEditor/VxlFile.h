#pragma once

#include <vector>
#include <memory>
#include <numeric>
#include <atlbase.h>
#include <cstdint>

#include <d3d11.h>
#include <DirectXMath.h>

// float has to obey IEEE 32-bit floating point rules
static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24);

struct Voxel
{
    uint8_t Color;
    uint8_t Normal;
};

struct VoxelSpan
{
    size_t Count;
    std::vector<Voxel> Voxels;
};

struct CCMatrix
{
    union
    {
        struct
        {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
        };
        float Data[3][4];
    };
};
static_assert(sizeof(CCMatrix) == 48);

class VxlFile
{
#pragma pack(push, 1)
    struct HvaStruct
    {
        uint8_t Signature[16];
        uint32_t FrameCount;
        uint32_t SectionCount;
        std::vector<CCMatrix> Matrices;
    };

    struct VxlLayerHeader
    {
        uint8_t LayerName[16];
        uint32_t LayerIndex;
        uint32_t Reserved1;
        uint32_t Reserved2;
    };
    static_assert(sizeof(VxlLayerHeader) == 28);

    struct VxlLayerInfo
    {
        uint32_t SpanStartOffset;
        uint32_t SpanEndOffset;
        uint32_t SpanDataOffset;
        float Scale;
        CCMatrix Matrix;
        float MinBounds[3];
        float MaxBounds[3];
        uint8_t SizeX;
        uint8_t SizeY;
        uint8_t SizeZ;
        uint8_t NormalType;
    };
    static_assert(sizeof(VxlLayerInfo) == 92);

    struct VxlHeader
    {
        int8_t Name[16]; // "Voxel Animation "
        uint32_t PaletteCount;
        uint32_t LayerCount;
        uint32_t LayerInfoCount;
        uint32_t BodySize;
        uint8_t RemapStartIndex;
        uint8_t RemapEndIndex;
        uint8_t EmbeddedPalette[256][3];
    };
    static_assert(sizeof(VxlHeader) == 802);
#pragma pack(pop)

    struct VxlLayerBody
    {
        std::vector<uint32_t> SpanStarts;
        std::vector<uint32_t> SpanEnds;
        std::vector<VoxelSpan> SpanData;
    };

public:
    // e.g. gtnk, without extension name
    explicit VxlFile(const char* vxlname);
    
    // e.g. gtnk.vxl & gtnk.hva, with extension name
    explicit VxlFile(const char* vxlfile, const char* hvafile);
    
    // This constructor doesn't make VxlFile own the buffer, remeber to relese them
    // We didn't check the size in fact, the size here is used to make a difference
    // with the last vxl file in fact
    explicit VxlFile(const void* vxlbuffer, size_t vxlsize,
        const void* hvabuffer, size_t hvasize);

    bool IsTextureLoaded() const;
    bool LoadTexture(ID3D11Device* device);
    void ClearTextures();

private:
    VxlHeader Header;
    std::vector<VxlLayerHeader> LayerHeaders;
    std::vector<VxlLayerBody> LayerBodies;
    std::vector<VxlLayerInfo> LayerInfos;
    HvaStruct Hva;
    CComPtr<ID3D11Texture3D> Texture;
};