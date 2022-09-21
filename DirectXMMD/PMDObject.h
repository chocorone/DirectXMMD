#pragma once
#include <vector>
#include "ShaderStruct.h"
#include "Output.h"

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

class PMDObject
{
public:
	PMDHeader header;
	unsigned int vertNum;
	unsigned int indicesNum;
	std::vector<unsigned char> vertics;
	std::vector<unsigned short> indices;

private:
	char *FilePass;
	char signature[3] = {};

public:
	bool LoadData(const char *pass);
};