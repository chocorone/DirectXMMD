#include "PMDObject.h"

bool PMDObject::LoadData(const char *pass)
{

	FilePass = (char *)pass;
	auto fp = fopen(FilePass, "rb");

	//ヘッダーの読み込み
	fread(signature, sizeof(signature), 1, fp);
	fread(&header, sizeof(PMDHeader), 1, fp);

	//頂点数の読み込み
	fread(&vertNum, sizeof(vertNum), 1, fp);

	//頂点データの読み込み
	vertics.resize(vertNum * pmdvertex_size);
	fread(vertics.data(), vertics.size(), 1, fp);

	//インデックス数の読み込み
	fread(&indicesNum, sizeof(indicesNum), 1, fp);

	//インデックスデータの読み込み
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	fclose(fp);
	OutputDebugFormatedString("size of pmd vertex = %zu\n", pmdvertex_size);
	OutputDebugStringA(header.model_name);
	OutputDebugFormatedString("\n");
	OutputDebugStringA(header.comment);
	OutputDebugFormatedString("\n");
	OutputDebugFormatedString("頂点数：%d\n", vertNum);
	OutputDebugFormatedString("インデックス数：%d\n", indicesNum);

	return true;
}
