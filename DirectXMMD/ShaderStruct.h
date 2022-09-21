#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "ComPtr.h"

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

struct TexRGBA
{
	unsigned char R, G, B, A;
};

struct CameraPos
{
	DirectX::XMFLOAT3 eye;
	DirectX::XMFLOAT3 target;
	DirectX::XMFLOAT3 up;
	float NearZ;
	float FarZ;
};

constexpr size_t pmdvertex_size = 38;