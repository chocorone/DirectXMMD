#include "Engine.h"

Engine* g_Engine;

bool Engine::Init(HWND hwnd, UINT windowWidth, UINT windowHight)
{
	OutputDebugString(TEXT("D3D�̏�������\n"));
	if (!CreateDevice()) {
		OutputDebugString(TEXT("�f�o�C�X�̐����Ɏ��s\n"));
		return false;
	}

	OutputDebugString(TEXT("D3D�̏������ɐ���\n"));
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
