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

struct PMDHeader
{
	float version;
	char model_name[20];
	char comment[256];
};

struct PMDVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	unsigned short boneNo[2];
	unsigned char edgeFlg;
};