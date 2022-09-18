#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "ComPtr.h"

struct Vertex 
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};