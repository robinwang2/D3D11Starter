#pragma once

#include <DirectXMath.h>

struct VertexShaderConstants
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT3 positionOffset;
	float padding;
};
