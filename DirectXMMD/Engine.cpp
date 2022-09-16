#include "Engine.h"

Engine* g_Engine;

bool Engine::Init(HWND hwnd, UINT windowWidth, UINT windowHight)
{
	OutputDebugString(TEXT("D3Dの初期化中\n"));
	if (!CreateDevice()) {
		OutputDebugString(TEXT("デバイスの生成に失敗\n"));
		return false;
	}

	OutputDebugString(TEXT("D3Dの初期化に成功\n"));
	return true;
}

bool Engine::CreateDevice()
{
	LRESULT res = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_dev));

	return SUCCEEDED(res);
}
