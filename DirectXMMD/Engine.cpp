#include "Engine.h"
#include "App.h"

#ifdef _DEBUG
#define OutputDebugFormatedString(str, ...)       \
	{                                             \
		TCHAR c[256];                             \
		swprintf(c, 256, TEXT(str), __VA_ARGS__); \
		OutputDebugString(c);                     \
	}
#else
#define MyOutputDebugString(str, ...) // 空実装
#endif

RenderingEngine *g_Engine;

bool RenderingEngine::Init(HWND hwnd)
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

	if (!CreateRTV())
	{
		OutputDebugString(TEXT("レンダーターゲットビューの生成に失敗\n"));
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

bool RenderingEngine::CreateDevice()
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

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0};

	//使用できるフィーチャーレベルの確認
	for (D3D_FEATURE_LEVEL l : levels)
	{
		LRESULT res = D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_device));

		if (SUCCEEDED(res))
		{

			return true;
		}
	}

	return false;
}

bool RenderingEngine::CreateCommandQueue()
{
	//コマンドアロケーターの作成
	LRESULT res = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (FAILED(res))
	{
		OutputDebugString(TEXT("コマンドアロケーターの作成に失敗\n"));
		return false;
	}

	//コマンドリストの作成
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

	//コマンドキューの生成
	res = _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmdQueue));

	if (FAILED(res))
	{
		OutputDebugString(TEXT("コマンドキューの作成に失敗\n"));
		return false;
	}

	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);

	return true;
}

bool RenderingEngine::CreateSwapChain(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = WINDOW_WIDTH;
	desc.Height = WINDOW_HEIGHT;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Stereo = false;
	//マルチサンプルの指定
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	desc.BufferCount = FRAME_BUFFER_COUNT;
	desc.Scaling = DXGI_SCALING_STRETCH;			 //伸び縮み可能
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //フリップ後は破棄
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //フルスクリーン切り替え可

	//スワップチェーンの作成
	LRESULT res = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		hWnd,
		&desc,
		nullptr,
		nullptr,
		(IDXGISwapChain1 **)_swapchain.GetAddressOf());

	return SUCCEEDED(res);
}

bool RenderingEngine::CreateRTV()
{
	// RTV用ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NodeMask = 0;
	desc.NumDescriptors = FRAME_BUFFER_COUNT;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// rtv用ディスクリプタヒープの作成
	LRESULT res = _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvHeaps));

	if (FAILED(res))
		return false;

	//スワップチェーンのメモリとrtv用ディスクリプタとの関連付け
	DXGI_SWAP_CHAIN_DESC swc_desc = {};
	res = _swapchain->GetDesc(&swc_desc);

	if (FAILED(res))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (int i = 0; i < swc_desc.BufferCount; i++)
	{
		res = _swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		if (FAILED(res))
			break;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		handle.ptr += i * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, handle);
	}

	return SUCCEEDED(res);
}

bool RenderingEngine::CreateFence()
{
	//フェンスの作成
	LRESULT res = _device->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	return SUCCEEDED(res);
}

void RenderingEngine::EnableDebugLayer()
{
	ID3D12Debug *debug = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		debug->EnableDebugLayer();
		debug->Release();
	}
}

bool RenderingEngine::beginRender()
{
	OutputDebugFormatedString("描画開始\n");

	//現在のバッファをレンダーターゲットビューに指定
	auto bbIndex = _swapchain->GetCurrentBackBufferIndex();
	_nowRTVDescripterHandle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	_nowRTVDescripterHandle.ptr += bbIndex * _device->GetDescriptorHandleIncrementSize(
												 D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//リソースバリアの設定
	_barriorDesc = {};
	_barriorDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	_barriorDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	_barriorDesc.Transition.pResource = backBuffers[bbIndex].Get();
	_barriorDesc.Transition.Subresource = 0;

	//レンダーターゲットとして設定
	_barriorDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	_barriorDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &_barriorDesc);

	//レンダーターゲットビューにセット
	_cmdList->OMSetRenderTargets(1, &_nowRTVDescripterHandle, true, nullptr);

	CreateRootSignature();
	CreateGraphicsPipelineState();
	CreateViewports();
	CreateScissorRect();

	//画面のクリア処理
	float clearColor[] = {0.8f, 0.7f, 1.0f, 1.0f};
	_cmdList->ClearRenderTargetView(_nowRTVDescripterHandle, clearColor, 0, nullptr);

	return true;
}

void RenderingEngine::endRender()
{
	// Present状態として設定
	_barriorDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_barriorDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &_barriorDesc);

	//コマンドキューの実行
	_cmdList->Close();
	ID3D12CommandList *cmdLists[] = {_cmdList.Get()};
	_cmdQueue->ExecuteCommandLists(1, cmdLists);

	//フェンスを使用して実行終了まで待機
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);

		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}

	//コマンドリスト・アロケーターのリセット
	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);

	//フリップ、垂直同期あり
	_swapchain->Present(1, 0);

	OutputDebugString(L"描画完了\n");
}

bool RenderingEngine::CreateRootSignature()
{
	//ディスクリプタレンジの設定
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメーターの設定
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	//サンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	//ルートシグニチャの作成
	D3D12_ROOT_SIGNATURE_DESC rootSignitureDesc = {};
	rootSignitureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignitureDesc.pParameters = &rootparam;
	rootSignitureDesc.NumParameters = 1;
	rootSignitureDesc.pStaticSamplers = &samplerDesc;
	rootSignitureDesc.NumStaticSamplers = 1;

	ID3DBlob *rootSigBlob = nullptr;
	ID3DBlob *errorBlob = nullptr;
	LRESULT res = D3D12SerializeRootSignature(
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

	res = _device->CreateRootSignature(0,
									   rootSigBlob->GetBufferPointer(),
									   rootSigBlob->GetBufferSize(),
									   IID_PPV_ARGS(&_rootSignature));

	rootSigBlob->Release();
	_cmdList->SetGraphicsRootSignature(_rootSignature.Get());
	return true;
}

bool RenderingEngine::CreateGraphicsPipelineState()
{

	ID3DBlob *vsBlob = nullptr;
	ID3DBlob *psBlob = nullptr;

	ID3DBlob *errorBlob = nullptr;

	LRESULT res = D3DCompileFromFile(
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

	//グラフィックスパイプラインステートの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = _rootSignature.Get();

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

	//頂点入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0,
		 DXGI_FORMAT_R32G32B32_FLOAT, 0,
		 D3D12_APPEND_ALIGNED_ELEMENT,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0,
		 DXGI_FORMAT_R32G32_FLOAT, 0,
		 D3D12_APPEND_ALIGNED_ELEMENT,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

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
	if (FAILED(res))
	{
		OutputDebugFormatedString("グラフィックスパイプラインの作成に失敗\n");
		return false;
	}
	//コマンドリストに追加
	_cmdList->SetPipelineState(_pipelineState.Get());
	return true;
}

void RenderingEngine::CreateViewports()
{
	//ビューポートの作成
	D3D12_VIEWPORT viewport = {};

	//出力先の幅と高さ
	viewport.Width = WINDOW_WIDTH;
	viewport.Height = WINDOW_HEIGHT;
	//出力先の左上座標
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	//深度の最大値・最小値
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	_cmdList->RSSetViewports(1, &viewport);
}

void RenderingEngine::CreateScissorRect()
{
	//シザー矩形の作成
	D3D12_RECT scissorrect = {};
	//切り抜き座標
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + WINDOW_WIDTH;
	scissorrect.bottom = scissorrect.top + WINDOW_HEIGHT;

	_cmdList->RSSetScissorRects(1, &scissorrect);
}

bool RenderingEngine::CreateVertexBufferView(const Vertex *vertices, const int vertNum, D3D12_VERTEX_BUFFER_VIEW *vbView)
{

	//頂点ヒープの設定
	D3D12_HEAP_PROPERTIES heapprp = {};
	heapprp.Type = D3D12_HEAP_TYPE_UPLOAD; // mapするためUPLOAD
	heapprp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソースの設定
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices[0]) * vertNum;
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//バッファー作成
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

	// Mapをしてバッファにデータをコピー
	Vertex *vertMap = nullptr;
	res = vertBuff->Map(0, nullptr, (void **)&vertMap);
	if (FAILED(res))
	{
		OutputDebugString(TEXT("頂点バッファのマッピングに失敗しました\n"));
		return false;
	}

	std::copy(vertices, vertices + vertNum, vertMap);
	vertBuff->Unmap(0, nullptr);

	vbView->BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView->StrideInBytes = sizeof(vertices[0]);
	vbView->SizeInBytes = sizeof(vertices[0]) * vertNum;

	return true;
}

bool RenderingEngine::CreateIndexBufferView(D3D12_INDEX_BUFFER_VIEW *ibView)
{
	//頂点ヒープの設定
	D3D12_HEAP_PROPERTIES heapprp = {};
	heapprp.Type = D3D12_HEAP_TYPE_UPLOAD; // mapするためUPLOAD
	heapprp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソースの設定
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(indices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//バッファー作成
	ID3D12Resource *idxBuff = nullptr;

	LRESULT res = _device->CreateCommittedResource(
		&heapprp,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));
	if (FAILED(res))
	{
		OutputDebugString(TEXT("インデックスバッファの作成に失敗しました\n"));
		return false;
	}

	//データをコピー
	unsigned short *mappedIdx = nullptr;
	res = idxBuff->Map(0, nullptr, (void **)&mappedIdx);
	if (FAILED(res))
	{
		OutputDebugString(TEXT("インデックスバッファのマッピングに失敗しました\n"));
		return false;
	}
	std::copy(indices, indices + 6, mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//インデックスバッファービューを作成
	ibView->BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView->Format = DXGI_FORMAT_R16_UINT;
	ibView->SizeInBytes = sizeof(indices);

	return true;
}

bool RenderingEngine::CreateTexShaderResourceView(std::vector<TexRGBA> texData)
{
	// WriteToSubresourceで転送するためのヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //転送はCPU側から直接
	heapProp.CreationNodeMask = 0;
	heapProp.VisibleNodeMask = 0;

	//リソースの設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	resDesc.Width = 256;
	resDesc.Height = 256;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//テクスチャバッファの作成
	ID3D12Resource *texBuff = nullptr;
	LRESULT res = _device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuff));
	if (FAILED(res))
	{
		OutputDebugFormatedString("テクスチャバッファの作成に失敗\n");
		return false;
	}

	// WriteToSubresourceによる転送
	res = texBuff->WriteToSubresource(
		0,
		nullptr,
		texData.data(),
		sizeof(TexRGBA) * 256,
		sizeof(TexRGBA) * texData.size());
	if (FAILED(res))
	{
		OutputDebugFormatedString("WriteToSubresourceによる転送に失敗\n");
		return false;
	}

	//ディスクリプタヒープの作成
	ID3D12DescriptorHeap *texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	res = _device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));

	//シェーダーリソースビューの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	//シェーダーリソースビューをディスクリプタヒープ上に作成
	_device->CreateShaderResourceView(
		texBuff,
		&srvDesc,
		texDescHeap->GetCPUDescriptorHandleForHeapStart());
	_cmdList->SetGraphicsRootSignature(_rootSignature.Get());
	_cmdList->SetDescriptorHeaps(1, &texDescHeap);
	_cmdList->SetGraphicsRootDescriptorTable(0, texDescHeap->GetGPUDescriptorHandleForHeapStart());
	return true;
}

bool RenderingEngine::CreateTexShaderResourceView(DirectX::TexMetadata texData, const DirectX::Image *img)
{
	// WriteToSubresourceで転送するためのヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //転送はCPU側から直接
	heapProp.CreationNodeMask = 0;
	heapProp.VisibleNodeMask = 0;

	//リソースの設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = texData.format;
	resDesc.Width = texData.width;
	resDesc.Height = texData.height;
	resDesc.DepthOrArraySize = texData.arraySize;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = texData.mipLevels;
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(texData.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//テクスチャバッファの作成
	ID3D12Resource *texBuff = nullptr;
	LRESULT res = _device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuff));
	if (FAILED(res))
	{
		OutputDebugFormatedString("テクスチャバッファの作成に失敗\n");
		return false;
	}

	// WriteToSubresourceによる転送
	res = texBuff->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		img->rowPitch,
		img->slicePitch);
	if (FAILED(res))
	{
		OutputDebugFormatedString("WriteToSubresourceによる転送に失敗\n");
		return false;
	}

	//ディスクリプタヒープの作成
	ID3D12DescriptorHeap *texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	res = _device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));

	//シェーダーリソースビューの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	//シェーダーリソースビューをディスクリプタヒープ上に作成
	_device->CreateShaderResourceView(
		texBuff,
		&srvDesc,
		texDescHeap->GetCPUDescriptorHandleForHeapStart());
	_cmdList->SetGraphicsRootSignature(_rootSignature.Get());
	_cmdList->SetDescriptorHeaps(1, &texDescHeap);
	_cmdList->SetGraphicsRootDescriptorTable(0, texDescHeap->GetGPUDescriptorHandleForHeapStart());
	return true;
}

bool RenderingEngine::RenderPolygon(Vertex *vertices, int vertNum, DirectX::TexMetadata texData, const DirectX::Image *img)
{
	//頂点バッファービューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	if (!CreateVertexBufferView(vertices, vertNum, &vbView))
	{
		OutputDebugFormatedString("頂点バッファービューの作成に失敗\n");
		return false;
	}

	//四角形ならインデックスバッファービューの作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	if (vertNum == 4)
	{
		if (!CreateIndexBufferView(&ibView))
		{
			OutputDebugFormatedString("インデックスバッファービューの作成に失敗\n");
			return false;
		}
	}

	if (!CreateTexShaderResourceView(texData, img))
	{
		OutputDebugFormatedString("テクスチャデータの作成に失敗\n");
		return false;
	}

	//コマンドリストに追加
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	if (vertNum == 3)
		_cmdList->DrawInstanced(3, 1, 0, 0);
	else if (vertNum == 4)
	{
		//四角形ならインデックスバッファをコマンドリストに追加
		_cmdList->IASetIndexBuffer(&ibView);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	OutputDebugFormatedString("ポリゴンの描画をコマンドリストに追加\n");
	return true;
}

bool RenderingEngine::RenderPolygon(Vertex *vertices, int vertNum, std::vector<TexRGBA> texData)
{
	//頂点バッファービューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	if (!CreateVertexBufferView(vertices, vertNum, &vbView))
	{
		OutputDebugFormatedString("頂点バッファービューの作成に失敗\n");
		return false;
	}

	//四角形ならインデックスバッファービューの作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	if (vertNum == 4)
	{
		if (!CreateIndexBufferView(&ibView))
		{
			OutputDebugFormatedString("インデックスバッファービューの作成に失敗\n");
			return false;
		}
	}

	if (!CreateTexShaderResourceView(texData))
	{
		OutputDebugFormatedString("テクスチャデータの作成に失敗\n");
		return false;
	}

	//コマンドリストに追加
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	if (vertNum == 3)
		_cmdList->DrawInstanced(3, 1, 0, 0);
	else if (vertNum == 4)
	{
		//四角形ならインデックスバッファをコマンドリストに追加
		_cmdList->IASetIndexBuffer(&ibView);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	OutputDebugFormatedString("ポリゴンの描画をコマンドリストに追加\n");
	return true;
}

bool RenderingEngine::SampleRender()
{
	beginRender();

	//ポリゴンの描画
	Vertex *vertices = new Vertex[4];
	vertices[0] = {{-0.4f, -0.7f, 0.0f}, {0.0f, 1.0f}};
	vertices[1] = {{-0.4f, 0.7f, 0.0f}, {0.0f, 0.0f}};
	vertices[2] = {{0.4f, -0.7f, 0.0f}, {1.0f, 1.0f}};
	vertices[3] = {{0.4f, 0.7f, 0.0f}, {1.0f, 0.0f}};

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};

	LRESULT res = LoadFromWICFile(L"./img/sampleTex.png", DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, scratchImg);

	if (FAILED(res))
	{
		OutputDebugFormatedString("テクスチャの読み込みに失敗");
	}
	auto img = scratchImg.GetImage(0, 0, 0);

	std::vector<TexRGBA> texData(256 * 256);
	for (int i = 0; i < 256 * 256; i++)
	{
		texData[i].R = i / 256;
		texData[i].G = i % 256;
		texData[i].B = 255;
		texData[i].A = 255;
	}
	if (!RenderPolygon(vertices, 4, metadata, img))
	{
		OutputDebugFormatedString("ポリゴンのレンダリングに失敗\n");
		return false;
	}

	// CreateGraphicsPipelineState();
	Vertex *vertices2 = new Vertex[3];
	vertices2[0] = {{-0.7f, -0.5f, 0.0f}, {0.0f, 1.0f}};
	vertices2[1] = {{-0.7f, 0.5f, 0.0f}, {0.0f, 0.0f}};
	vertices2[2] = {{-0.4f, -0.5f, 0.0f}, {1.0f, 1.0f}};

	if (!RenderPolygon(vertices2, 3, texData))
	{
		OutputDebugFormatedString("2kaimenoポリゴンのレンダリングに失敗\n");
		return false;
	}
	endRender();

	return true;
}