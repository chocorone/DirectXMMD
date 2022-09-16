#include "Engine.h"
#include "App.h"

Engine* g_Engine;

bool Engine::Init(HWND hwnd, UINT windowWidth, UINT windowHight)
{
	OutputDebugString(TEXT("D3Dの初期化中\n"));
	if (!CreateDevice()) {
		OutputDebugString(TEXT("デバイスの生成に失敗\n"));
		return false;
	}

	if (!CreateDXGIFactory()) {
		OutputDebugString(TEXT("アダプターの設定に失敗\n"));
		return false;
	}

	if (!CreateCommandQueue()) {
		OutputDebugString(TEXT("コマンドキューの生成に失敗\n"));
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

	//とりあえず1つめを選択
	tmpAdapter = adapters[0];

	//ここでアダプターの選択ができる
	/*for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;
	}*/
	return true;
}

bool Engine::CreateCommandQueue()
{
	LRESULT res = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (FAILED(res)) {
		OutputDebugString(TEXT("コマンドアロケーターの作成に失敗\n"));
		return false;
	}

	res = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (FAILED(res)) {
		OutputDebugString(TEXT("コマンドリストの作成に失敗\n"));
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//タイムアウトなし
	desc.NodeMask =0 ;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	res = _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmdQueue));

	if (FAILED(res)) {
		OutputDebugString(TEXT("コマンドキューの作成に失敗\n"));
		return false;
	}

	return true;
}

bool Engine::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = WINDOW_WIDTH;
	desc.Height = WINDOW_HEIGHT;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	desc.BufferCount = 2;

	desc.Scaling = DXGI_SCALING_STRETCH;

	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	LRESULT res = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		*g_hWnd,
		&desc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain
	);

	if (FAILED(res)) {
		return false;
	}

	return false;
}
