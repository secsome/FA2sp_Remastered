#include "StdAfx.h"

#include "Convert.h"

#include "CCFile.h"

#include <numbers>
#include <cmath>

static constexpr RGBStruct HSV2RGB(HSVStruct hsv)
{
    const auto& [H, S, V] = hsv;

    if (S == 0)
        return { V, V, V };

    const unsigned char region = H / 43;
    const unsigned char remainder = (H - (region * 43)) * 6;

    const unsigned char p = (V * (255 - S)) >> 8;
    const unsigned char q = (V * (255 - ((S * remainder) >> 8))) >> 8;
    const unsigned char t = (V * (255 - ((S * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0: return { V, t, p };
        case 1: return { q, V, p };
        case 2: return { p, V, t };
        case 3: return { p, q, V };
        case 4: return { t, p, V };
        default: return { V, p, q };
    }
}

static constexpr HSVStruct RGB2HSV(RGBStruct rgb)
{
	int hue, saturation, value;
    const auto& [red, green, blue] = rgb;

	hue = 0;

    value = red > green ? red : green;
    if (blue > value) value = blue;

    int white = red < green ? red : green;
    if (blue < white) white = blue;

	saturation = 0;
	if (value)
		saturation = ((value - white) * 255) / value;

	if (saturation != 0)
	{
		unsigned int tmp = value - white;
		unsigned int r1 = ((value - red) * 255) / tmp;
		unsigned int g1 = ((value - green) * 255) / tmp;
		unsigned int b1 = ((value - blue) * 255) / tmp;

		if (value == red)
			if (white == green)
				tmp = 5 * 256 + b1;
			else
				tmp = 1 * 256 - g1;
		else
			if (value == green)
				if (white == blue)
					tmp = 1 * 256 + r1;
				else
					tmp = 3 * 256 - b1;
			else
				if (white == red)
					tmp = 3 * 256 + g1;
				else
					tmp = 5 * 256 - r1;
		hue = tmp / 6;
	}

	return { (uint8_t)hue, (uint8_t)saturation, (uint8_t)value };
}

ConvertClass::ConvertClass(const char* filename)
{
    CCFileClass file{ filename };
    if (file.Size() != sizeof(Palette))
        throw std::runtime_error("Invalid palette size");
    if (file.Read(Palette.data(), sizeof(Palette)) != sizeof(Palette))
        throw std::runtime_error("Failed to read palette");
    file.Close();
    for (auto& [r, g, b] : Palette)
    {
        r <<= 2;
        g <<= 2;
        b <<= 2;
    }
    OriginalPalette = Palette;
}

ConvertClass::ConvertClass(const void* buffer, size_t size)
{
    if (size != sizeof(ConvertClass))
        throw std::runtime_error("Invalid palette size");
    std::memcpy(Palette.data(), buffer, size);
    for (auto& [r, g, b] : Palette)
    {
        r <<= 2;
        g <<= 2;
        b <<= 2;
    }
    OriginalPalette = Palette;
}

void ConvertClass::Remap(RGBStruct color)
{
    for (size_t i = RemapStart; i < RemapEnd; ++i)
    {
        size_t off = i - RemapStart;
        double vcos = off * 0.08144869842640204 + 0.3490658503988659;
        double vsin = off * 0.04654211338651545 + 0.8726646259971648;
        if (off == 0)
            vcos = 0.1963495408493621;

        HSVStruct hsv = RGB2HSV(OriginalPalette[i]);
        hsv.S = static_cast<uint8_t>(std::sin(vsin) * hsv.S);
        hsv.V = static_cast<uint8_t>(std::cos(vcos) * hsv.V);
        Palette[i] = HSV2RGB(hsv);
    }
}
