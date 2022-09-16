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
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	for (D3D_FEATURE_LEVEL l : levels) {
		LRESULT res = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&_dev));

		if (SUCCEEDED(res)) {
			
			return true;
		}
	}

	return false;
}
