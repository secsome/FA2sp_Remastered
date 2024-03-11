#pragma once

// Well, YR only have a fixed camera, so I don't think we need to implement a camera class

#include <DirectXMath.h>

class GameCamera
{
public:


private:
    DirectX::XMFLOAT3 m_scale;
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
};