#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <d3d12sdklayers.h>
#include <vector>
#include <string>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Engine
{
	//�ϐ��̒�`
private:
	ID3D12Device *_device = nullptr;
	IDXGIFactory6 *_dxgiFactory = nullptr;
	IDXGISwapChain4 *_swapchain = nullptr;
	ID3D12CommandAllocator *_cmdAllocator = nullptr;
	ID3D12GraphicsCommandList *_cmdList = nullptr;
	ID3D12CommandQueue *_cmdQueue = nullptr;
	ID3D12DescriptorHeap *_rtvHeaps = nullptr;
	std::vector<ID3D12Resource *> backBuffers;
	ID3D12Fence *_fence = nullptr;
	UINT _fenceVal = 0;

	D3D_FEATURE_LEVEL featureLevel;

	//�֐��̒�`
public:
	bool Init(HWND hwnd); //�G���W��������
	void SampleRender();

private:				 // DirectX12�������Ɏg���֐�
	bool CreateDevice(); //�f�o�C�X�𐶐�
	bool CreateDXGIFactory();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hWnd);
	bool CreateDescriptorHeap();
	bool CreateFence();
	void EnableDebugLayer();
};

extern Engine *g_Engine;