#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <d3d12sdklayers.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "ComPtr.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Engine
{
//変数の定義
public:
	enum { FRAME_BUFFER_COUNT = 2 };

private:
	ComPtr<ID3D12Device> _device = nullptr;
	ComPtr<ID3D12GraphicsCommandList>_cmdList = nullptr;
	ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	ComPtr<ID3D12Resource> backBuffers[FRAME_BUFFER_COUNT] = { nullptr };
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT _fenceVal = 0;
	D3D_FEATURE_LEVEL featureLevel;

	//関数の定義
public:
	bool Init(HWND hwnd); //エンジン初期化
	void SampleRender();
	void SanmplePolygonRender(DirectX::XMFLOAT3 vertics[]);

private:				 // DirectX12初期化に使う関数
	bool CreateDevice(); //デバイスを生成
	bool CreateDXGIFactory();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hWnd);
	bool CreateDescriptorHeap();
	bool CreateFence();
	void EnableDebugLayer();
};

extern Engine *g_Engine;