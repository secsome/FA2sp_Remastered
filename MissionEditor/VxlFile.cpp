#include "StdAfx.h"

#include "VxlFile.h"

#include "CCFile.h"
#include "RenderHelper.h"

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
                if (voxel_count != end_sig)
                    throw std::runtime_error("The VXL file seems corrupted");
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

const NormalTableType& VxlFile::GetNormalTable(size_t index) const
{
    const static NormalTableType NormalTable1 {
        DirectX::XMVectorSet(0.54946297f, -0.000183f, -0.835518f, 0.0f),
        DirectX::XMVectorSet(0.00014400001f, 0.54940403f, -0.83555698f, 0.0f),
        DirectX::XMVectorSet(-0.54940403f, -0.000068000001f, -0.83555698f, 0.0f),
        DirectX::XMVectorSet(0.000106f, -0.54946297f, -0.835518f, 0.0f),
        DirectX::XMVectorSet(0.94900799f, 0.00031599999f, -0.31525001f, 0.0f),
        DirectX::XMVectorSet(-0.000186f, 0.94899702f, -0.31528401f, 0.0f),
        DirectX::XMVectorSet(-0.94899702f, 0.00031800001f, -0.31528401f, 0.0f),
        DirectX::XMVectorSet(-0.000447f, -0.94900799f, -0.31525001f, 0.0f),
        DirectX::XMVectorSet(0.95084399f, -0.000279f, 0.30967101f, 0.0f),
        DirectX::XMVectorSet(0.000202f, 0.95084798f, 0.30965701f, 0.0f),
        DirectX::XMVectorSet(-0.95084798f, -0.000070000002f, 0.30965701f, 0.0f),
        DirectX::XMVectorSet(0.000147f, -0.95084399f, 0.30967101f, 0.0f),
        DirectX::XMVectorSet(0.55237001f, -0.000011f, 0.83359897f, 0.0f),
        DirectX::XMVectorSet(0.000019999999f, 0.55238003f, 0.833592f, 0.0f),
        DirectX::XMVectorSet(-0.55238003f, 0.000057000001f, 0.83359301f, 0.0f),
        DirectX::XMVectorSet(-0.000066000001f, -0.55237001f, 0.83359897f, 0.0f)
    };

    const static NormalTableType NormalTable2 {
        DirectX::XMVectorSet(0.67121398f, 0.19849201f, -0.714194f, 0.0f),
        DirectX::XMVectorSet(0.26964301f, 0.58439398f, -0.76536f, 0.0f),
        DirectX::XMVectorSet(-0.040546f, 0.096988f, -0.99445897f, 0.0f),
        DirectX::XMVectorSet(-0.57242799f, -0.091913998f, -0.81478697f, 0.0f),
        DirectX::XMVectorSet(-0.17140099f, -0.57270998f, -0.80163902f, 0.0f),
        DirectX::XMVectorSet(0.36255699f, -0.30299899f, -0.88133103f, 0.0f),
        DirectX::XMVectorSet(0.81034702f, -0.34897199f, -0.470698f, 0.0f),
        DirectX::XMVectorSet(0.103962f, 0.93867201f, -0.328767f, 0.0f),
        DirectX::XMVectorSet(-0.324047f, 0.58766901f, -0.74137598f, 0.0f),
        DirectX::XMVectorSet(-0.80086499f, 0.34046099f, -0.49264699f, 0.0f),
        DirectX::XMVectorSet(-0.66549802f, -0.59014702f, -0.45698899f, 0.0f),
        DirectX::XMVectorSet(0.314767f, -0.803002f, -0.506073f, 0.0f),
        DirectX::XMVectorSet(0.97262901f, 0.151076f, -0.17655f, 0.0f),
        DirectX::XMVectorSet(0.680291f, 0.68423599f, -0.26272699f, 0.0f),
        DirectX::XMVectorSet(-0.52007902f, 0.82777703f, -0.210483f, 0.0f),
        DirectX::XMVectorSet(-0.96164399f, -0.179001f, -0.207847f, 0.0f),
        DirectX::XMVectorSet(-0.262714f, -0.937451f, -0.22840101f, 0.0f),
        DirectX::XMVectorSet(0.219707f, -0.97130102f, 0.091124997f, 0.0f),
        DirectX::XMVectorSet(0.92380798f, -0.229975f, 0.30608699f, 0.0f),
        DirectX::XMVectorSet(-0.082488999f, 0.97065997f, 0.225866f, 0.0f),
        DirectX::XMVectorSet(-0.59179801f, 0.69678998f, 0.40528899f, 0.0f),
        DirectX::XMVectorSet(-0.92529601f, 0.36660099f, 0.097111002f, 0.0f),
        DirectX::XMVectorSet(-0.705051f, -0.68777502f, 0.172828f, 0.0f),
        DirectX::XMVectorSet(0.7324f, -0.68036699f, -0.026304999f, 0.0f),
        DirectX::XMVectorSet(0.85516202f, 0.37458199f, 0.358311f, 0.0f),
        DirectX::XMVectorSet(0.47300601f, 0.83648002f, 0.276705f, 0.0f),
        DirectX::XMVectorSet(-0.097617f, 0.65411198f, 0.750072f, 0.0f),
        DirectX::XMVectorSet(-0.90412402f, -0.153725f, 0.39865801f, 0.0f),
        DirectX::XMVectorSet(-0.211916f, -0.85808998f, 0.46773201f, 0.0f),
        DirectX::XMVectorSet(0.50022697f, -0.67440802f, 0.543091f, 0.0f),
        DirectX::XMVectorSet(0.584539f, -0.110249f, 0.80384099f, 0.0f),
        DirectX::XMVectorSet(0.43737301f, 0.45464399f, 0.77588898f, 0.0f),
        DirectX::XMVectorSet(-0.042440999f, 0.083318003f, 0.995619f, 0.0f),
        DirectX::XMVectorSet(-0.59625101f, 0.22013199f, 0.77202803f, 0.0f),
        DirectX::XMVectorSet(-0.506455f, -0.39697701f, 0.76544899f, 0.0f),
        DirectX::XMVectorSet(0.070569001f, -0.47847399f, 0.87526202f, 0.0f)
    };

    const static NormalTableType NormalTable3 {
        DirectX::XMVectorSet(0.45651099f, -0.073968001f, -0.88663799f, 0.0f),
        DirectX::XMVectorSet(0.50769401f, 0.38511699f, -0.77067f, 0.0f),
        DirectX::XMVectorSet(0.095431998f, 0.22666401f, -0.96928602f, 0.0f),
        DirectX::XMVectorSet(-0.35876599f, 0.54318798f, -0.75910097f, 0.0f),
        DirectX::XMVectorSet(-0.361276f, 0.13299499f, -0.92292601f, 0.0f),
        DirectX::XMVectorSet(-0.48311701f, -0.32406601f, -0.813375f, 0.0f),
        DirectX::XMVectorSet(-0.018073f, -0.197559f, -0.980124f, 0.0f),
        DirectX::XMVectorSet(0.3211f, -0.501477f, -0.80337799f, 0.0f),
        DirectX::XMVectorSet(0.79949099f, 0.069615997f, -0.59662998f, 0.0f),
        DirectX::XMVectorSet(0.390971f, 0.77130598f, -0.50222403f, 0.0f),
        DirectX::XMVectorSet(0.080782004f, 0.61448997f, -0.784778f, 0.0f),
        DirectX::XMVectorSet(-0.73275f, 0.41143101f, -0.54203498f, 0.0f),
        DirectX::XMVectorSet(-0.73525399f, 0.0091019999f, -0.67773098f, 0.0f),
        DirectX::XMVectorSet(-0.80249399f, -0.39490801f, -0.44727099f, 0.0f),
        DirectX::XMVectorSet(-0.13413f, -0.58915502f, -0.79680902f, 0.0f),
        DirectX::XMVectorSet(0.71955299f, -0.37622699f, -0.58369303f, 0.0f),
        DirectX::XMVectorSet(0.96687502f, 0.173593f, -0.187132f, 0.0f),
        DirectX::XMVectorSet(0.760831f, 0.51910597f, -0.38944301f, 0.0f),
        DirectX::XMVectorSet(-0.114642f, 0.87551898f, -0.46938601f, 0.0f),
        DirectX::XMVectorSet(-0.53236699f, 0.76885903f, -0.354177f, 0.0f),
        DirectX::XMVectorSet(-0.96226698f, 0.024977f, -0.27095801f, 0.0f),
        DirectX::XMVectorSet(-0.46738699f, -0.721986f, -0.51018202f, 0.0f),
        DirectX::XMVectorSet(0.058449998f, -0.85235399f, -0.51968902f, 0.0f),
        DirectX::XMVectorSet(0.49823299f, -0.74374002f, -0.44566301f, 0.0f),
        DirectX::XMVectorSet(0.93915099f, -0.27024499f, -0.212044f, 0.0f),
        DirectX::XMVectorSet(0.58393198f, 0.80944198f, -0.061857f, 0.0f),
        DirectX::XMVectorSet(0.183797f, 0.97322798f, -0.138007f, 0.0f),
        DirectX::XMVectorSet(-0.88435501f, 0.45221901f, -0.115822f, 0.0f),
        DirectX::XMVectorSet(-0.943178f, -0.33206701f, 0.012138f, 0.0f),
        DirectX::XMVectorSet(-0.69844002f, -0.70656699f, -0.113772f, 0.0f),
        DirectX::XMVectorSet(-0.228411f, -0.95470601f, -0.190694f, 0.0f),
        DirectX::XMVectorSet(0.73156399f, -0.675861f, -0.089588001f, 0.0f),
        DirectX::XMVectorSet(0.96925098f, 0.046804f, 0.24158201f, 0.0f),
        DirectX::XMVectorSet(0.85564703f, 0.50347698f, 0.119916f, 0.0f),
        DirectX::XMVectorSet(-0.25115299f, 0.96794701f, -0.000080999998f, 0.0f),
        DirectX::XMVectorSet(-0.64779502f, 0.75674897f, 0.087711997f, 0.0f),
        DirectX::XMVectorSet(-0.96916401f, 0.14519399f, 0.1991f, 0.0f),
        DirectX::XMVectorSet(-0.41479301f, -0.88896698f, 0.194126f, 0.0f),
        DirectX::XMVectorSet(0.25077501f, -0.961178f, -0.115109f, 0.0f),
        DirectX::XMVectorSet(0.47862899f, -0.84259301f, 0.246883f, 0.0f),
        DirectX::XMVectorSet(0.89004397f, -0.39614201f, 0.225595f, 0.0f),
        DirectX::XMVectorSet(0.52405101f, 0.76235998f, 0.37970701f, 0.0f),
        DirectX::XMVectorSet(0.11962f, 0.94548202f, 0.30291f, 0.0f),
        DirectX::XMVectorSet(-0.76085001f, 0.49007499f, 0.42536199f, 0.0f),
        DirectX::XMVectorSet(-0.86978501f, -0.20215f, 0.450122f, 0.0f),
        DirectX::XMVectorSet(-0.70946699f, -0.60242403f, 0.36570701f, 0.0f),
        DirectX::XMVectorSet(0.019308999f, -0.95887101f, 0.28318599f, 0.0f),
        DirectX::XMVectorSet(0.626113f, -0.564677f, 0.53770101f, 0.0f),
        DirectX::XMVectorSet(0.769943f, -0.126663f, 0.62541503f, 0.0f),
        DirectX::XMVectorSet(0.76419097f, 0.35070199f, 0.54131401f, 0.0f),
        DirectX::XMVectorSet(-0.001878f, 0.74136698f, 0.67109799f, 0.0f),
        DirectX::XMVectorSet(-0.37088001f, 0.81836802f, 0.43900099f, 0.0f),
        DirectX::XMVectorSet(-0.71390897f, 0.12865201f, 0.68831801f, 0.0f),
        DirectX::XMVectorSet(-0.295165f, -0.73866397f, 0.60601401f, 0.0f),
        DirectX::XMVectorSet(0.186195f, -0.73836899f, 0.648184f, 0.0f),
        DirectX::XMVectorSet(0.387523f, -0.35878301f, 0.84917599f, 0.0f),
        DirectX::XMVectorSet(0.481022f, 0.124846f, 0.86777401f, 0.0f),
        DirectX::XMVectorSet(0.391808f, 0.54505599f, 0.741216f, 0.0f),
        DirectX::XMVectorSet(-0.0035359999f, 0.36559799f, 0.93076599f, 0.0f),
        DirectX::XMVectorSet(-0.42049801f, 0.484961f, 0.76680797f, 0.0f),
        DirectX::XMVectorSet(-0.35490301f, 0.019470001f, 0.93470001f, 0.0f),
        DirectX::XMVectorSet(-0.54783702f, -0.35920799f, 0.75554299f, 0.0f),
        DirectX::XMVectorSet(-0.106662f, -0.445115f, 0.88909799f, 0.0f),
        DirectX::XMVectorSet(0.086796001f, -0.059307002f, 0.99445897f, 0.0f)
    };

    const static NormalTableType NormalTable4 {
        DirectX::XMVectorSet(0.52657801f, -0.35962099f, -0.77031702f, 0.0f),
        DirectX::XMVectorSet(0.150482f, 0.43598399f, 0.88728398f, 0.0f),
        DirectX::XMVectorSet(0.414195f, 0.73825502f, -0.53237402f, 0.0f),
        DirectX::XMVectorSet(0.075152002f, 0.91624898f, -0.393498f, 0.0f),
        DirectX::XMVectorSet(-0.316149f, 0.93073601f, -0.18379299f, 0.0f),
        DirectX::XMVectorSet(-0.77381903f, 0.62333399f, -0.11251f, 0.0f),
        DirectX::XMVectorSet(-0.90084201f, 0.42853701f, -0.069568001f, 0.0f),
        DirectX::XMVectorSet(-0.99894202f, -0.010971f, 0.044665001f, 0.0f),
        DirectX::XMVectorSet(-0.979761f, -0.15767001f, -0.123324f, 0.0f),
        DirectX::XMVectorSet(-0.91127402f, -0.362371f, -0.19562f, 0.0f),
        DirectX::XMVectorSet(-0.62406898f, -0.72094101f, -0.301301f, 0.0f),
        DirectX::XMVectorSet(-0.310173f, -0.80934501f, -0.498752f, 0.0f),
        DirectX::XMVectorSet(0.146613f, -0.81581903f, -0.55941403f, 0.0f),
        DirectX::XMVectorSet(-0.71651602f, -0.69435602f, -0.066887997f, 0.0f),
        DirectX::XMVectorSet(0.50397199f, -0.114202f, -0.85613698f, 0.0f),
        DirectX::XMVectorSet(0.45549101f, 0.87262702f, -0.176211f, 0.0f),
        DirectX::XMVectorSet(-0.00501f, -0.114373f, -0.99342501f, 0.0f),
        DirectX::XMVectorSet(-0.104675f, -0.327701f, -0.93896502f, 0.0f),
        DirectX::XMVectorSet(0.56041199f, 0.75258899f, -0.34575599f, 0.0f),
        DirectX::XMVectorSet(-0.060575999f, 0.82162797f, -0.566796f, 0.0f),
        DirectX::XMVectorSet(-0.30234101f, 0.79700702f, -0.522847f, 0.0f),
        DirectX::XMVectorSet(-0.671543f, 0.67074001f, -0.314863f, 0.0f),
        DirectX::XMVectorSet(-0.77840102f, -0.12835699f, 0.61450499f, 0.0f),
        DirectX::XMVectorSet(-0.92404997f, 0.278382f, -0.261985f, 0.0f),
        DirectX::XMVectorSet(-0.69977301f, -0.55049098f, -0.45527801f, 0.0f),
        DirectX::XMVectorSet(-0.56824797f, -0.51718903f, -0.64000797f, 0.0f),
        DirectX::XMVectorSet(0.054097999f, -0.93286401f, -0.356143f, 0.0f),
        DirectX::XMVectorSet(0.75838202f, 0.57289302f, -0.31088799f, 0.0f),
        DirectX::XMVectorSet(0.0036200001f, 0.30502599f, -0.95233703f, 0.0f),
        DirectX::XMVectorSet(-0.060849998f, -0.98688602f, -0.14951099f, 0.0f),
        DirectX::XMVectorSet(0.63523f, 0.045478001f, -0.77098298f, 0.0f),
        DirectX::XMVectorSet(0.52170497f, 0.241309f, -0.81828701f, 0.0f),
        DirectX::XMVectorSet(0.26940399f, 0.63542497f, -0.72364098f, 0.0f),
        DirectX::XMVectorSet(0.045676f, 0.67275399f, -0.738455f, 0.0f),
        DirectX::XMVectorSet(-0.180511f, 0.67465699f, -0.71571898f, 0.0f),
        DirectX::XMVectorSet(-0.397131f, 0.63664001f, -0.66104198f, 0.0f),
        DirectX::XMVectorSet(-0.55200398f, 0.47251499f, -0.687038f, 0.0f),
        DirectX::XMVectorSet(-0.77217001f, 0.08309f, -0.62996f, 0.0f),
        DirectX::XMVectorSet(-0.669819f, -0.119533f, -0.73284f, 0.0f),
        DirectX::XMVectorSet(-0.54045498f, -0.31844401f, -0.77878201f, 0.0f),
        DirectX::XMVectorSet(-0.38613501f, -0.522789f, -0.75999397f, 0.0f),
        DirectX::XMVectorSet(-0.261466f, -0.68856698f, -0.676395f, 0.0f),
        DirectX::XMVectorSet(-0.019412f, -0.69610298f, -0.71767998f, 0.0f),
        DirectX::XMVectorSet(0.30356899f, -0.48184401f, -0.82199299f, 0.0f),
        DirectX::XMVectorSet(0.68193901f, -0.19512901f, -0.70490003f, 0.0f),
        DirectX::XMVectorSet(-0.24488901f, -0.116562f, -0.96251899f, 0.0f),
        DirectX::XMVectorSet(0.80075902f, -0.022979001f, -0.59854603f, 0.0f),
        DirectX::XMVectorSet(-0.37027499f, 0.095583998f, -0.92399102f, 0.0f),
        DirectX::XMVectorSet(-0.33067101f, -0.32657799f, -0.88543999f, 0.0f),
        DirectX::XMVectorSet(-0.16322f, -0.52757901f, -0.83367902f, 0.0f),
        DirectX::XMVectorSet(0.12639f, -0.313146f, -0.941257f, 0.0f),
        DirectX::XMVectorSet(0.34954801f, -0.27222601f, -0.89649802f, 0.0f),
        DirectX::XMVectorSet(0.23991799f, -0.085825004f, -0.96699202f, 0.0f),
        DirectX::XMVectorSet(0.390845f, 0.081537001f, -0.91683799f, 0.0f),
        DirectX::XMVectorSet(0.25526699f, 0.26869699f, -0.92878503f, 0.0f),
        DirectX::XMVectorSet(0.146245f, 0.48043799f, -0.86474901f, 0.0f),
        DirectX::XMVectorSet(-0.32601601f, 0.47845599f, -0.81534898f, 0.0f),
        DirectX::XMVectorSet(-0.46968201f, -0.112519f, -0.87563598f, 0.0f),
        DirectX::XMVectorSet(0.81844002f, -0.25852001f, -0.51315099f, 0.0f),
        DirectX::XMVectorSet(-0.474318f, 0.292238f, -0.83043301f, 0.0f),
        DirectX::XMVectorSet(0.778943f, 0.39584199f, -0.48637101f, 0.0f),
        DirectX::XMVectorSet(0.62409401f, 0.39377299f, -0.67487001f, 0.0f),
        DirectX::XMVectorSet(0.74088597f, 0.203834f, -0.63995302f, 0.0f),
        DirectX::XMVectorSet(0.48021701f, 0.565768f, -0.67029703f, 0.0f),
        DirectX::XMVectorSet(0.38093001f, 0.42453501f, -0.82137799f, 0.0f),
        DirectX::XMVectorSet(-0.093422003f, 0.50112402f, -0.86031801f, 0.0f),
        DirectX::XMVectorSet(-0.236485f, 0.29619801f, -0.92538702f, 0.0f),
        DirectX::XMVectorSet(-0.131531f, 0.093959004f, -0.98684901f, 0.0f),
        DirectX::XMVectorSet(-0.82356203f, 0.29577699f, -0.48400599f, 0.0f),
        DirectX::XMVectorSet(0.61106598f, -0.624304f, -0.486664f, 0.0f),
        DirectX::XMVectorSet(0.069495998f, -0.52033001f, -0.85113299f, 0.0f),
        DirectX::XMVectorSet(0.226522f, -0.66487902f, -0.711775f, 0.0f),
        DirectX::XMVectorSet(0.47130799f, -0.56890398f, -0.67395699f, 0.0f),
        DirectX::XMVectorSet(0.38842499f, -0.74262398f, -0.54556f, 0.0f),
        DirectX::XMVectorSet(0.78367501f, -0.48072901f, -0.39338499f, 0.0f),
        DirectX::XMVectorSet(0.962394f, 0.135676f, -0.235349f, 0.0f),
        DirectX::XMVectorSet(0.876607f, 0.172034f, -0.449406f, 0.0f),
        DirectX::XMVectorSet(0.63340503f, 0.58979303f, -0.50094098f, 0.0f),
        DirectX::XMVectorSet(0.182276f, 0.80065799f, -0.57072097f, 0.0f),
        DirectX::XMVectorSet(0.177003f, 0.76413399f, 0.62029701f, 0.0f),
        DirectX::XMVectorSet(-0.544016f, 0.675515f, -0.49772099f, 0.0f),
        DirectX::XMVectorSet(-0.67929697f, 0.28646699f, -0.67564201f, 0.0f),
        DirectX::XMVectorSet(-0.59039098f, 0.091369003f, -0.801929f, 0.0f),
        DirectX::XMVectorSet(-0.82436001f, -0.13312399f, -0.55018902f, 0.0f),
        DirectX::XMVectorSet(-0.71579403f, -0.33454201f, -0.61296099f, 0.0f),
        DirectX::XMVectorSet(0.17428599f, -0.89248401f, 0.416049f, 0.0f),
        DirectX::XMVectorSet(-0.082528003f, -0.83712298f, -0.54075301f, 0.0f),
        DirectX::XMVectorSet(0.28333101f, -0.88087398f, -0.37918901f, 0.0f),
        DirectX::XMVectorSet(0.675134f, -0.42662701f, -0.60181701f, 0.0f),
        DirectX::XMVectorSet(0.84372002f, -0.512335f, -0.160156f, 0.0f),
        DirectX::XMVectorSet(0.97730398f, -0.098555997f, -0.18752f, 0.0f),
        DirectX::XMVectorSet(0.846295f, 0.522672f, -0.102947f, 0.0f),
        DirectX::XMVectorSet(0.67714101f, 0.72132498f, -0.145501f, 0.0f),
        DirectX::XMVectorSet(0.32096499f, 0.87089199f, -0.37219399f, 0.0f),
        DirectX::XMVectorSet(-0.178978f, 0.911533f, -0.37023601f, 0.0f),
        DirectX::XMVectorSet(-0.44716901f, 0.82670099f, -0.341474f, 0.0f),
        DirectX::XMVectorSet(-0.70320302f, 0.496328f, -0.50908101f, 0.0f),
        DirectX::XMVectorSet(-0.97718102f, 0.063562997f, -0.202674f, 0.0f),
        DirectX::XMVectorSet(-0.87817001f, -0.412938f, 0.241455f, 0.0f),
        DirectX::XMVectorSet(-0.83583099f, -0.35855001f, -0.415728f, 0.0f),
        DirectX::XMVectorSet(-0.499174f, -0.69343299f, -0.51959199f, 0.0f),
        DirectX::XMVectorSet(-0.188789f, -0.92375302f, -0.33322501f, 0.0f),
        DirectX::XMVectorSet(0.19225401f, -0.96936101f, -0.152896f, 0.0f),
        DirectX::XMVectorSet(0.51594001f, -0.783907f, -0.34539199f, 0.0f),
        DirectX::XMVectorSet(0.90592498f, -0.30095199f, -0.29787099f, 0.0f),
        DirectX::XMVectorSet(0.99111199f, -0.127746f, 0.037106998f, 0.0f),
        DirectX::XMVectorSet(0.99513501f, 0.098424003f, -0.0043830001f, 0.0f),
        DirectX::XMVectorSet(0.76012301f, 0.64627701f, 0.067367002f, 0.0f),
        DirectX::XMVectorSet(0.205221f, 0.95958f, -0.192591f, 0.0f),
        DirectX::XMVectorSet(-0.042750001f, 0.97951299f, -0.19679099f, 0.0f),
        DirectX::XMVectorSet(-0.43801701f, 0.89892697f, 0.0084920004f, 0.0f),
        DirectX::XMVectorSet(-0.82199401f, 0.48078501f, -0.30523899f, 0.0f),
        DirectX::XMVectorSet(-0.89991701f, 0.081710003f, -0.42833701f, 0.0f),
        DirectX::XMVectorSet(-0.92661202f, -0.144618f, -0.347096f, 0.0f),
        DirectX::XMVectorSet(-0.79365999f, -0.55779201f, -0.24283899f, 0.0f),
        DirectX::XMVectorSet(-0.43134999f, -0.84777898f, -0.30855799f, 0.0f),
        DirectX::XMVectorSet(-0.0054919999f, -0.96499997f, 0.26219299f, 0.0f),
        DirectX::XMVectorSet(0.58790499f, -0.80402601f, -0.088940002f, 0.0f),
        DirectX::XMVectorSet(0.69949299f, -0.66768599f, -0.254765f, 0.0f),
        DirectX::XMVectorSet(0.88930303f, 0.359795f, -0.282291f, 0.0f),
        DirectX::XMVectorSet(0.780972f, 0.197037f, 0.59267199f, 0.0f),
        DirectX::XMVectorSet(0.52012098f, 0.50669599f, 0.68755698f, 0.0f),
        DirectX::XMVectorSet(0.40389499f, 0.69396102f, 0.59605998f, 0.0f),
        DirectX::XMVectorSet(-0.154983f, 0.89923602f, 0.40909001f, 0.0f),
        DirectX::XMVectorSet(-0.65733802f, 0.53716803f, 0.528543f, 0.0f),
        DirectX::XMVectorSet(-0.74619502f, 0.33409101f, 0.575827f, 0.0f),
        DirectX::XMVectorSet(-0.62495202f, -0.049144f, 0.77911502f, 0.0f),
        DirectX::XMVectorSet(0.31814101f, -0.254715f, 0.913185f, 0.0f),
        DirectX::XMVectorSet(-0.555897f, 0.405294f, 0.725752f, 0.0f),
        DirectX::XMVectorSet(-0.79443401f, 0.099405997f, 0.59916002f, 0.0f),
        DirectX::XMVectorSet(-0.64036101f, -0.68946302f, 0.33849499f, 0.0f),
        DirectX::XMVectorSet(-0.12671299f, -0.73409498f, 0.66711998f, 0.0f),
        DirectX::XMVectorSet(0.105457f, -0.78081697f, 0.61579502f, 0.0f),
        DirectX::XMVectorSet(0.40799299f, -0.48091599f, 0.77605498f, 0.0f),
        DirectX::XMVectorSet(0.69513601f, -0.54512f, 0.468647f, 0.0f),
        DirectX::XMVectorSet(0.97319102f, -0.0064889998f, 0.229908f, 0.0f),
        DirectX::XMVectorSet(0.94689399f, 0.317509f, -0.050799001f, 0.0f),
        DirectX::XMVectorSet(0.56358302f, 0.82561201f, 0.027183f, 0.0f),
        DirectX::XMVectorSet(0.325773f, 0.94542301f, 0.0069490001f, 0.0f),
        DirectX::XMVectorSet(-0.171821f, 0.98509699f, -0.0078149997f, 0.0f),
        DirectX::XMVectorSet(-0.67044097f, 0.73993897f, 0.054768998f, 0.0f),
        DirectX::XMVectorSet(-0.822981f, 0.55496198f, 0.121322f, 0.0f),
        DirectX::XMVectorSet(-0.96619302f, 0.117857f, 0.229307f, 0.0f),
        DirectX::XMVectorSet(-0.95376903f, -0.29470399f, 0.058945f, 0.0f),
        DirectX::XMVectorSet(-0.86438698f, -0.50272799f, -0.010015f, 0.0f),
        DirectX::XMVectorSet(-0.53060901f, -0.84200603f, -0.097365998f, 0.0f),
        DirectX::XMVectorSet(-0.162618f, -0.98407501f, 0.071772002f, 0.0f),
        DirectX::XMVectorSet(0.081446998f, -0.99601102f, 0.036439002f, 0.0f),
        DirectX::XMVectorSet(0.74598402f, -0.66596299f, 0.00076199998f, 0.0f),
        DirectX::XMVectorSet(0.94205701f, -0.32926899f, -0.064106002f, 0.0f),
        DirectX::XMVectorSet(0.93970197f, -0.28108999f, 0.194803f, 0.0f),
        DirectX::XMVectorSet(0.77121401f, 0.55067003f, 0.319363f, 0.0f),
        DirectX::XMVectorSet(0.641348f, 0.73069f, 0.23402099f, 0.0f),
        DirectX::XMVectorSet(0.080682002f, 0.99669099f, 0.0098789996f, 0.0f),
        DirectX::XMVectorSet(-0.046725001f, 0.97664303f, 0.20972501f, 0.0f),
        DirectX::XMVectorSet(-0.53107601f, 0.82100099f, 0.209562f, 0.0f),
        DirectX::XMVectorSet(-0.69581503f, 0.65599f, 0.29243499f, 0.0f),
        DirectX::XMVectorSet(-0.97612202f, 0.216709f, -0.014913f, 0.0f),
        DirectX::XMVectorSet(-0.96166098f, -0.14412899f, 0.23331399f, 0.0f),
        DirectX::XMVectorSet(-0.772084f, -0.61364698f, 0.165299f, 0.0f),
        DirectX::XMVectorSet(-0.44960001f, -0.83605999f, 0.314426f, 0.0f),
        DirectX::XMVectorSet(-0.39269999f, -0.91461599f, 0.096247002f, 0.0f),
        DirectX::XMVectorSet(0.390589f, -0.91947001f, 0.044890001f, 0.0f),
        DirectX::XMVectorSet(0.58252901f, -0.79919797f, 0.148127f, 0.0f),
        DirectX::XMVectorSet(0.866431f, -0.48981199f, 0.096864f, 0.0f),
        DirectX::XMVectorSet(0.90458697f, 0.111498f, 0.41145f, 0.0f),
        DirectX::XMVectorSet(0.95353699f, 0.23232999f, 0.191806f, 0.0f),
        DirectX::XMVectorSet(0.497311f, 0.77080297f, 0.398177f, 0.0f),
        DirectX::XMVectorSet(0.194066f, 0.95631999f, 0.218611f, 0.0f),
        DirectX::XMVectorSet(0.422876f, 0.882276f, 0.206797f, 0.0f),
        DirectX::XMVectorSet(-0.373797f, 0.84956598f, 0.37217399f, 0.0f),
        DirectX::XMVectorSet(-0.53449702f, 0.71402299f, 0.4522f, 0.0f),
        DirectX::XMVectorSet(-0.881827f, 0.23716f, 0.40759799f, 0.0f),
        DirectX::XMVectorSet(-0.904948f, -0.014069f, 0.42528901f, 0.0f),
        DirectX::XMVectorSet(-0.751827f, -0.51281703f, 0.41445801f, 0.0f),
        DirectX::XMVectorSet(-0.50101501f, -0.69791698f, 0.51175803f, 0.0f),
        DirectX::XMVectorSet(-0.23519f, -0.92592299f, 0.295555f, 0.0f),
        DirectX::XMVectorSet(0.228983f, -0.95393997f, 0.193819f, 0.0f),
        DirectX::XMVectorSet(0.734025f, -0.63489801f, 0.241062f, 0.0f),
        DirectX::XMVectorSet(0.91375297f, -0.063253f, -0.40131599f, 0.0f),
        DirectX::XMVectorSet(0.90573502f, -0.161487f, 0.391875f, 0.0f),
        DirectX::XMVectorSet(0.85892999f, 0.342446f, 0.38074899f, 0.0f),
        DirectX::XMVectorSet(0.62448603f, 0.60758102f, 0.49077699f, 0.0f),
        DirectX::XMVectorSet(0.28926399f, 0.85747898f, 0.42550799f, 0.0f),
        DirectX::XMVectorSet(0.069968f, 0.90216899f, 0.42567101f, 0.0f),
        DirectX::XMVectorSet(-0.28617999f, 0.94069999f, 0.182165f, 0.0f),
        DirectX::XMVectorSet(-0.57401299f, 0.80511898f, -0.14930899f, 0.0f),
        DirectX::XMVectorSet(0.111258f, 0.099717997f, -0.98877603f, 0.0f),
        DirectX::XMVectorSet(-0.30539301f, -0.94422799f, -0.12316f, 0.0f),
        DirectX::XMVectorSet(-0.60116601f, -0.78957599f, 0.123163f, 0.0f),
        DirectX::XMVectorSet(-0.290645f, -0.81213999f, 0.50591898f, 0.0f),
        DirectX::XMVectorSet(-0.064920001f, -0.87716299f, 0.47578499f, 0.0f),
        DirectX::XMVectorSet(0.408301f, -0.862216f, 0.29978901f, 0.0f),
        DirectX::XMVectorSet(0.56609702f, -0.72556603f, 0.39126399f, 0.0f),
        DirectX::XMVectorSet(0.83936399f, -0.427387f, 0.33586901f, 0.0f),
        DirectX::XMVectorSet(0.81889999f, -0.041305002f, 0.57244802f, 0.0f),
        DirectX::XMVectorSet(0.71978402f, 0.41499701f, 0.55649698f, 0.0f),
        DirectX::XMVectorSet(0.88174403f, 0.45027f, 0.140659f, 0.0f),
        DirectX::XMVectorSet(0.40182301f, -0.89822f, -0.17815199f, 0.0f),
        DirectX::XMVectorSet(-0.054019999f, 0.79134399f, 0.60898f, 0.0f),
        DirectX::XMVectorSet(-0.29377401f, 0.76399398f, 0.57446498f, 0.0f),
        DirectX::XMVectorSet(-0.450798f, 0.61034697f, 0.65135098f, 0.0f),
        DirectX::XMVectorSet(-0.63822103f, 0.186694f, 0.74687302f, 0.0f),
        DirectX::XMVectorSet(-0.87287003f, -0.25712699f, 0.41470799f, 0.0f),
        DirectX::XMVectorSet(-0.58725703f, -0.52170998f, 0.618828f, 0.0f),
        DirectX::XMVectorSet(-0.35365799f, -0.64197397f, 0.680291f, 0.0f),
        DirectX::XMVectorSet(0.041648999f, -0.61127299f, 0.79032302f, 0.0f),
        DirectX::XMVectorSet(0.348342f, -0.77918297f, 0.52108699f, 0.0f),
        DirectX::XMVectorSet(0.499167f, -0.62244099f, 0.602826f, 0.0f),
        DirectX::XMVectorSet(0.79001898f, -0.30383101f, 0.53250003f, 0.0f),
        DirectX::XMVectorSet(0.66011798f, 0.060733002f, 0.74870199f, 0.0f),
        DirectX::XMVectorSet(0.60492098f, 0.29416099f, 0.73996001f, 0.0f),
        DirectX::XMVectorSet(0.38569701f, 0.37934601f, 0.84103203f, 0.0f),
        DirectX::XMVectorSet(0.239693f, 0.207876f, 0.94833201f, 0.0f),
        DirectX::XMVectorSet(0.012623f, 0.25853199f, 0.96591997f, 0.0f),
        DirectX::XMVectorSet(-0.100557f, 0.457147f, 0.88368797f, 0.0f),
        DirectX::XMVectorSet(0.046967f, 0.62858802f, 0.77631903f, 0.0f),
        DirectX::XMVectorSet(-0.43039101f, -0.44540501f, 0.785097f, 0.0f),
        DirectX::XMVectorSet(-0.43429101f, -0.196228f, 0.87913901f, 0.0f),
        DirectX::XMVectorSet(-0.25663701f, -0.336867f, 0.90590203f, 0.0f),
        DirectX::XMVectorSet(-0.131372f, -0.15891001f, 0.97851402f, 0.0f),
        DirectX::XMVectorSet(0.102379f, -0.208767f, 0.972592f, 0.0f),
        DirectX::XMVectorSet(0.195687f, -0.450129f, 0.87125802f, 0.0f),
        DirectX::XMVectorSet(0.62731898f, -0.42314801f, 0.65377098f, 0.0f),
        DirectX::XMVectorSet(0.68743902f, -0.171583f, 0.70568198f, 0.0f),
        DirectX::XMVectorSet(0.27592f, -0.021255f, 0.96094602f, 0.0f),
        DirectX::XMVectorSet(0.45936701f, 0.15746599f, 0.87417799f, 0.0f),
        DirectX::XMVectorSet(0.285395f, 0.583184f, 0.76055598f, 0.0f),
        DirectX::XMVectorSet(-0.81217402f, 0.46030301f, 0.35846099f, 0.0f),
        DirectX::XMVectorSet(-0.189068f, 0.64122301f, 0.743698f, 0.0f),
        DirectX::XMVectorSet(-0.338875f, 0.47648001f, 0.811252f, 0.0f),
        DirectX::XMVectorSet(-0.92099398f, 0.347186f, 0.176727f, 0.0f),
        DirectX::XMVectorSet(0.040638998f, 0.024465f, 0.99887401f, 0.0f),
        DirectX::XMVectorSet(-0.73913199f, -0.35374701f, 0.57318997f, 0.0f),
        DirectX::XMVectorSet(-0.60351199f, -0.28661501f, 0.74405998f, 0.0f),
        DirectX::XMVectorSet(-0.188676f, -0.547059f, 0.81555402f, 0.0f),
        DirectX::XMVectorSet(-0.026045f, -0.39782f, 0.91709399f, 0.0f),
        DirectX::XMVectorSet(0.26789701f, -0.649041f, 0.71202302f, 0.0f),
        DirectX::XMVectorSet(0.518246f, -0.28489101f, 0.80638599f, 0.0f),
        DirectX::XMVectorSet(0.493451f, -0.066532999f, 0.86722499f, 0.0f),
        DirectX::XMVectorSet(-0.328188f, 0.140251f, 0.93414301f, 0.0f),
        DirectX::XMVectorSet(0.328188f, 0.140251f, 0.93414301f, 0.0f),
        DirectX::XMVectorSet(-0.328188f, 0.140251f, 0.93414301f, 0.0f),
        DirectX::XMVectorSet(-0.328188f, 0.140251f, 0.93414301f, 0.0f),
        DirectX::XMVectorSet(-0.328188f, 0.140251f, 0.93414301f, 0.0f)
    };

    switch (LayerInfos[index].NormalType)
    {
    case 1:
        return NormalTable1;
    case 2:
        return NormalTable2;
    case 3:
        return NormalTable3;
    case 4:
        return NormalTable4;
    default:
        throw std::invalid_argument("Invalid NormalType");
    }
}

DirectX::XMMATRIX VxlFile::GetHvaMatrix(size_t section, size_t frame) const
{
    return Hva.Matrices[frame * (section - 1) + section];
}
