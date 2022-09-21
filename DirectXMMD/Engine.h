#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <d3d12sdklayers.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <wchar.h>
#include <DirectXTex.h>
#include "ComPtr.h"
#include "ShaderStruct.h"
#include "Output.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

class RenderingEngine
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
	ComPtr<ID3D12DescriptorHeap> _texDescHeap = nullptr;
	ComPtr<ID3D12Resource> backBuffers[FRAME_BUFFER_COUNT] = {nullptr};
	ComPtr<ID3D12Fence> _fence = nullptr;
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
	UINT _fenceVal = 0;
	D3D_FEATURE_LEVEL featureLevel;
	D3D12_RESOURCE_BARRIER _barriorDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _nowRTVDescripterHandle;
	ComPtr<ID3D12RootSignature> _rootSignature = nullptr;

	const unsigned short indices[6] = {
		0, 1, 2,
		2, 1, 3};

	float _angle = 0;

	//カメラの座標
	DirectX::XMFLOAT3 eye = {0, 10, -15};
	DirectX::XMFLOAT3 target = {0, 10, 0};
	DirectX::XMFLOAT3 up = {0, 1, 0};
	float NearZ = 1.0f;
	float FarZ = 100.0f;

	DirectX::TexMetadata whiteMetaData = {};
	DirectX::Image whiteImg = {};

public:
	bool Init(HWND hwnd);
	bool SampleRender(std::vector<unsigned char> vertices);
	void RotatePolygon(float angle);
	bool beginRender();
	void endRender();

private:
	bool CreateDevice(); //デバイス生成用関数
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hWnd);
	bool CreateRTV();
	bool CreateFence();

	bool CreatePipelineState();
	void CreateViewports();
	void CreateScissorRect();

	bool RenderPMD(std::vector<unsigned char> vertices);

	void EnableDebugLayer();
};

extern RenderingEngine *g_Engine;