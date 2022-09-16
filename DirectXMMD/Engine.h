#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")



class Engine {
//変数の定義
private:
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

//関数の定義
public:
	bool Init(HWND hwnd, UINT windowWidth, UINT windowHight);//エンジン初期化

private://DirectX12初期化に使う関数
	bool CreateDevice();//デバイスを生成


};

extern Engine* g_Engine;