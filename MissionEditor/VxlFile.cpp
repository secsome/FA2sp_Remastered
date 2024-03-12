#include "StdAfx.h"

#include "VxlFile.h"

#include "CCFile.h"
#include "GameTexture.h"

#include <algorithm>

VxlFile::VxlFile(const char* vxlname)
{
    std::string vxl = vxlname;
    std::string hva = vxlname;
    vxl += ".vxl";
    hva += ".hva";

    new (this) VxlFile{ vxl.c_str(), hva.c_str() };
}

VxlFile::VxlFile(const char* vxlfile, const char* hvafile)
{
    CCFileClass vxl{ vxlfile };
    size_t vxlsize = vxl.Size();
    const auto vxlbuffer = new char[vxlsize];
    if (vxl.Read(vxlbuffer, vxlsize) != vxlsize)
        throw std::runtime_error("Failed to read VXL file");
    vxl.Close();

    CCFileClass hva{ hvafile };
    size_t hvasize = hva.Size();
    const auto hvabuffer = new char[hvasize];
    if (hva.Read(hvabuffer, hvasize) != hvasize)
        throw std::runtime_error("Failed to read HVA file");
    hva.Close();

    new (this) VxlFile{ vxlbuffer, vxlsize, hvabuffer, hvasize };

    delete[] vxlbuffer;
    delete[] hvabuffer;
}

VxlFile::VxlFile(const void* vxlbuffer, size_t vxlsize, const void* hvabuffer, size_t hvasize)
{
    if (vxlbuffer == nullptr || vxlsize == 0 || hvabuffer == nullptr || hvasize == 0)
        throw std::invalid_argument("Empty buffer or size");

    auto ptr = static_cast<const uint8_t*>(vxlbuffer);
    auto ptr_end = ptr + vxlsize;

    std::memcpy(&Header, ptr, sizeof(VxlHeader));
    ptr += sizeof(VxlHeader);
    if (ptr >= ptr_end)
        throw std::runtime_error("Buffer overflow when loading VXL");

    for (auto& color : Header.EmbeddedPalette)
    {
        color[0] <<= 2;
        color[1] <<= 2;
        color[2] <<= 2;
    }

    if (Header.LayerCount != Header.LayerInfoCount)
        throw std::runtime_error("Cannot process LayerCount != LayerInfoCount VXL file");

    LayerHeaders.resize(Header.LayerCount);
    LayerInfos.resize(Header.LayerInfoCount);
    LayerBodies.resize(Header.LayerInfoCount);

    std::memcpy(LayerHeaders.data(), ptr, sizeof(VxlLayerHeader) * LayerHeaders.size());
    ptr += sizeof(VxlLayerHeader) * LayerHeaders.size();
    ptr += Header.BodySize;
    if (ptr >= ptr_end)
        throw std::runtime_error("Buffer overflow when loading VXL");
    std::memcpy(LayerInfos.data(), ptr, sizeof(VxlLayerInfo) * LayerInfos.size());
    ptr -= Header.BodySize;

    for (size_t n = 0; n < LayerBodies.size(); ++n)
    {
        auto& body = LayerBodies[n];
        const auto info = LayerInfos[n];

        const auto span_count = info.SizeX * info.SizeY;
        body.SpanStarts.resize(span_count);
        body.SpanEnds.resize(span_count);
        body.SpanData.resize(span_count);

        const auto data = ptr + info.SpanDataOffset;
        const auto starts = ptr + info.SpanStartOffset;
        const auto ends = ptr + info.SpanEndOffset;

        std::memcpy(body.SpanStarts.data(), starts, span_count * sizeof(uint32_t));
        std::memcpy(body.SpanEnds.data(), ends, span_count * sizeof(uint32_t));

        for (size_t i = 0; i < span_count; ++i)
        {
            auto& [count, voxels] = body.SpanData[i];
            count = 0;
            voxels.resize(info.SizeZ);

            if (body.SpanStarts[i] == 0xffffffff || body.SpanEnds[i] == 0xffffffff)
                continue;

            auto dst = voxels.data();
            for (auto p = data + body.SpanStarts[i]; p <= data + body.SpanEnds[i]; )
            {
                const auto skip_count = *p++;
                const auto voxel_count = *p++;
                dst += skip_count;

                if (voxel_count)
                {
                    std::memcpy(dst, p, voxel_count * sizeof(Voxel));
                    p += voxel_count * sizeof(Voxel);
                    dst += voxel_count;
                }

                const auto end_sig = *p++;
                // if (voxel_count != end_sig)
                //     throw std::runtime_error("The VXL file seems corrupted");
            }

            std::for_each(voxels.begin(), voxels.end(), [&count](const auto& voxel) {if (voxel.Color != 0) ++count; });
        }
    }

    // Load HVA data
    ptr = reinterpret_cast<const uint8_t*>(hvabuffer);
    ptr_end = ptr + hvasize;
    std::memcpy(&Hva, ptr, sizeof(Hva.Signature) + sizeof(Hva.FrameCount) + sizeof(Hva.SectionCount));
    ptr += sizeof(Hva.Signature) + sizeof(Hva.FrameCount) + sizeof(Hva.SectionCount);
    if (ptr >= ptr_end)
        throw std::runtime_error("Buffer overflow when loading HVA");
    Hva.Matrices.resize(Hva.FrameCount * Hva.SectionCount);
    std::memcpy(Hva.Matrices.data(), ptr, Hva.FrameCount * Hva.SectionCount * sizeof(CCMatrix));
    ptr += Hva.FrameCount * Hva.SectionCount * sizeof(CCMatrix);
    if (ptr >= ptr_end)
        throw std::runtime_error("Buffer overflow when loading HVA");
}

bool VxlFile::IsTextureLoaded() const
{
    return Texture != nullptr;
}

bool VxlFile::LoadTexture(ID3D11Device* device)
{
    if (device == nullptr)
        return false;

    return true;
}

void VxlFile::ClearTextures()
{
    Texture.Release();
}
