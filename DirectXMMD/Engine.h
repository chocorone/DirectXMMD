#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <d3d12sdklayers.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "ComPtr.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class Engine
{
public:
	enum
	{
		FRAME_BUFFER_COUNT = 2
	};

private:
	ComPtr<ID3D12Device> _device = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	ComPtr<ID3D12Resource> backBuffers[FRAME_BUFFER_COUNT] = {nullptr};
	ComPtr<ID3D12Fence> _fence = nullptr;
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
	UINT _fenceVal = 0;
	D3D_FEATURE_LEVEL featureLevel;

public:
	bool Init(HWND hwnd);
	void SampleRender();
	bool SanmplePolygonRender(DirectX::XMFLOAT3 *vertics);

private:
	bool CreateDevice();
	bool CreateDXGIFactory();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hWnd);
	bool CreateDescriptorHeap();
	bool CreateFence();
	void EnableDebugLayer();
};

extern Engine *g_Engine;