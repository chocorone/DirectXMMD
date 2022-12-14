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
#include <stdio.h>
#include <wchar.h>
#include <DirectXTex.h>
#include "ComPtr.h"
#include "ShaderStruct.h"

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

public:
	bool Init(HWND hwnd);
	bool SampleRender();

private:
	bool CreateDevice(); //デバイス生成用関数
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hWnd);
	bool CreateRTV();
	bool CreateFence();

	void EnableDebugLayer();
	bool beginRender();
	void endRender();
	void CreateViewports();
	void CreateScissorRect();
	bool CreateGraphicsPipelineState();
	bool CreateRootSignature();

	bool CreateVertexBufferView(const Vertex *vertices, const int vertNum, D3D12_VERTEX_BUFFER_VIEW *vbView);
	bool CreateIndexBufferView(D3D12_INDEX_BUFFER_VIEW *ibView);
	bool CreateTexShaderResourceView(std::vector<TexRGBA> texData);
	bool CreateTexShaderResourceView(DirectX::TexMetadata texData, const DirectX::Image* img);
	bool RenderPolygon(Vertex *vertices, int vertNum, std::vector<TexRGBA> texData);
	bool RenderPolygon(Vertex *vertices, int vertNum, DirectX::TexMetadata texData, const DirectX::Image* img);
};

extern RenderingEngine *g_Engine;