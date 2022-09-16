#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")



class Engine {
//�ϐ��̒�`
private:
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

//�֐��̒�`
public:
	bool Init(HWND hwnd, UINT windowWidth, UINT windowHight);//�G���W��������

private://DirectX12�������Ɏg���֐�
	bool CreateDevice();//�f�o�C�X�𐶐�


};

extern Engine* g_Engine;