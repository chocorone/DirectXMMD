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

bool Engine::SanmplePolygonRender(DirectX::XMFLOAT3 *vertics)
{
	//頂点バッファの作成
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
		return false;
	}

	DirectX::XMFLOAT3 *vertMap = nullptr;
	res = vertBuff->Map(0, nullptr, (void **)&vertMap);

	if (FAILED(res))
	{
		OutputDebugString(TEXT("マップの生成に失敗しました\n"));
		return false;
	}

	std::copy(vertics, vertics + 3, vertMap);

	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();

	vbView.SizeInBytes = sizeof(vertics);
	vbView.StrideInBytes = sizeof(vertics[0]);

	ID3DBlob *vsBlob = nullptr;
	ID3DBlob *psBlob = nullptr;

	ID3DBlob *errorBlob = nullptr;

	res = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errorBlob);

	if (FAILED(res))
	{
		if (errorBlob)
		{
			OutputDebugString(TEXT("頂点シェーダーコンパイルエラー:"));
			OutputDebugStringA(static_cast<char *>(errorBlob->GetBufferPointer()));
		}
		return false;
	}

	res = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob);

	if (FAILED(res))
	{
		if (errorBlob)
		{
			OutputDebugString(TEXT("ピクセルシェーダーコンパイルエラー:"));
			OutputDebugStringA(static_cast<char *>(errorBlob->GetBufferPointer()));
		}
		return false;
	}

	//頂点入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0,
		 DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		 D3D12_APPEND_ALIGNED_ELEMENT,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	//グラフィックスパイプラインステートの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	//ルートシグニチャの作成
	D3D12_ROOT_SIGNATURE_DESC rootSignitureDesc = {};
	rootSignitureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob *rootSigBlob = nullptr;
	res = D3D12SerializeRootSignature(
		&rootSignitureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);
	if (FAILED(res))
	{
		if (errorBlob)
		{
			OutputDebugString(TEXT("ルートシグニチャの生成に失敗:"));
			OutputDebugStringA(static_cast<char *>(errorBlob->GetBufferPointer()));
		}
		return false;
	}
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	res = _device->CreateRootSignature(0,
									   rootSigBlob->GetBufferPointer(),
									   rootSigBlob->GetBufferSize(),
									   IID_PPV_ARGS(&rootSignature));
	rootSigBlob->Release();
	gpipeline.pRootSignature = rootSignature.Get();

	//シェーダーの設定
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

	//サンプルマスクの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//アンチエイリアスの設定
	gpipeline.RasterizerState.MultisampleEnable = false;
	//カリングなし
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//中身を塗りつぶす
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//深度方向のクリッピング
	gpipeline.RasterizerState.DepthClipEnable = true;

	//ブレンドステートの設定
	//αテストの有無
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	//それぞれのレンダーゲットに別々のブレンドステートを割り当てるか
	gpipeline.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc = {};
	//加算、乗算、αなどのブレンドを行うか
	blenddesc.BlendEnable = false;
	//論理演算するか
	blenddesc.LogicOpEnable = false;
	//書き込むときのマスク値
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	//入力レイアウトの指定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//カットなし
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//プリミティブトポロジの指定
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//レンダーターゲットの指定
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

	//アンチエイジングのサンプル数設定
	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	//グラフィックスパイプラインの作成
	res = _device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelineState));

	if (SUCCEEDED(res))
	{
		OutputDebugString(TEXT("ok\n"));
	}
	return true;
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
