#include "Engine.h"

Engine* g_Engine;

bool Engine::Init(HWND hwnd, UINT windowWidth, UINT windowHight)
{
	OutputDebugString(TEXT("D3D�̏�������\n"));
	if (!CreateDevice()) {
		OutputDebugString(TEXT("�f�o�C�X�̐����Ɏ��s\n"));
		return false;
	}

	/*if (!CreateDXGIFactory()) {
		OutputDebugString(TEXT("�A�_�v�^�[�̐ݒ�Ɏ��s\n"));
		return false;
	}*/

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
		LRESULT res = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&_device));

		if (SUCCEEDED(res)) {
			
			return true;
		}
	}

	return false;
}

bool Engine::CreateDXGIFactory()
{
	LRESULT res = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	if (FAILED(res)) 
		return false;
	
	std::vector<IDXGIAdapter*> adapters;

	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}

	if (adapters.size() == 0)
		return false;

	//�Ƃ肠����1�߂�I��
	tmpAdapter = adapters[0];

	//�����ŃA�_�v�^�[�̑I�����ł���
	/*for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;
	}*/
	return true;
}
