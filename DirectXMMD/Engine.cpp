#include "Engine.h"
#include "App.h"

Engine *g_Engine;

bool Engine::Init(HWND hwnd)
{
#ifdef _DEBUG
	EnableDebugLayer();
#endif

	OutputDebugString(TEXT("D3Dの初期化中\n"));
	if (!CreateDevice())
	{
		OutputDebugString(TEXT("デバイスの生成に失敗\n"));
		return false;
	}

	if (!CreateDXGIFactory())
	{
		OutputDebugString(TEXT("アダプターの設定に失敗\n"));
		return false;
	}

	if (!CreateCommandQueue())
	{
		OutputDebugString(TEXT("コマンドキューの生成に失敗\n"));
		return false;
	}

	if (!CreateSwapChain(hwnd))
	{
		OutputDebugString(TEXT("スワップチェーンの生成に失敗\n"));
		return false;
	}

	if (!CreateDescriptorHeap())
	{
		OutputDebugString(TEXT("ディスクリプタヒープの生成に失敗\n"));
		return false;
	}
	if (!CreateFence())
	{
		OutputDebugString(TEXT("フェンスの生成に失敗\n"));
		return false;
	}

	OutputDebugString(TEXT("D3Dの初期化に成功\n"));

	return true;
}

void Engine::SampleRender()
{
	//スワップチェーンの動作確認
	//現在のバッファをレンダーターゲットビューに指定
	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);
	auto bbIndex = _swapchain->GetCurrentBackBufferIndex();
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIndex * _device->GetDescriptorHandleIncrementSize(
							  D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//ここにリソースバリアの設定
	D3D12_RESOURCE_BARRIER barriorDesc = {};
	barriorDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriorDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriorDesc.Transition.pResource = backBuffers[bbIndex].Get();
	barriorDesc.Transition.Subresource = 0;

	barriorDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barriorDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &barriorDesc);

	_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	//コマンドリストの作成
	float clearColor[] = {0.8f, 0.7f, 1.0f, 1.0f};
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	barriorDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barriorDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &barriorDesc);

	_cmdList->Close();
	ID3D12CommandList *cmdLists[] = {_cmdList.Get()};
	_cmdQueue->ExecuteCommandLists(1, cmdLists);

	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);

		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}
	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);

	//フリップ
	_swapchain->Present(1, 0);
}

void Engine::SanmplePolygonRender(DirectX::XMFLOAT3 vertics[])
{
	D3D12_HEAP_PROPERTIES heapprp = {};

	heapprp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertics);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource *vertBuff = nullptr;

	LRESULT res = _device->CreateCommittedResource(&heapprp,
												   D3D12_HEAP_FLAG_NONE,
												   &resdesc,
												   D3D12_RESOURCE_STATE_GENERIC_READ,
												   nullptr,
												   IID_PPV_ARGS(&vertBuff));

	if (FAILED(res))
	{
		OutputDebugString(TEXT("頂点バッファーの生成に失敗しました\n"));
	}
}

void Engine::EnableDebugLayer()
{
	ID3D12Debug *debug = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		debug->EnableDebugLayer();
		debug->Release();
	}
}

bool Engine::CreateDevice()
{
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0};

	for (D3D_FEATURE_LEVEL l : levels)
	{
		LRESULT res = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&_device));

		if (SUCCEEDED(res))
		{

			return true;
		}
	}

	return false;
}

bool Engine::CreateDXGIFactory()
{
#ifdef _DEBUG
	LRESULT res = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	LRESULT res = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	if (FAILED(res))
		return false;

	std::vector<IDXGIAdapter *> adapters;

	IDXGIAdapter *tmpAdapter = nullptr;

	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
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
	if (FAILED(res))
	{
		OutputDebugString(TEXT("コマンドアロケーターの作成に失敗\n"));
		return false;
	}

	res = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList));
	if (FAILED(res))
	{
		OutputDebugString(TEXT("コマンドリストの作成に失敗\n"));
		return false;
	}
	_cmdList->Close();

	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //タイムアウトなし
	desc.NodeMask = 0;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	res = _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmdQueue));

	if (FAILED(res))
	{
		OutputDebugString(TEXT("コマンドキューの作成に失敗\n"));
		return false;
	}

	return true;
}

bool Engine::CreateSwapChain(HWND hWnd)
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
		_cmdQueue.Get(),
		hWnd,
		&desc,
		nullptr,
		nullptr,
		(IDXGISwapChain1 **)_swapchain.GetAddressOf());

	return SUCCEEDED(res);
}

bool Engine::CreateDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NodeMask = 0;
	desc.NumDescriptors = 2;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	LRESULT res = _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvHeaps));

	if (FAILED(res))
		return false;

	DXGI_SWAP_CHAIN_DESC swc_desc = {};
	res = _swapchain->GetDesc(&swc_desc);

	if (FAILED(res))
		return false;

	for (int i = 0; i < swc_desc.BufferCount; i++)
	{
		res = _swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		if (FAILED(res))
			break;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		handle.ptr += i * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
	}

	return SUCCEEDED(res);
}

bool Engine::CreateFence()
{
	LRESULT res = _device->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	return SUCCEEDED(res);
}
