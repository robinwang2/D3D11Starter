#pragma once

#include <DirectXMath.h>
struct VertexShaderExternalData
{
	DirectX::XMFLOAT4 colorTint;    // 16 bytes 
	DirectX::XMFLOAT3 offset;       // 12 bytes 
	float padding;                  // 4 bytes  
};

