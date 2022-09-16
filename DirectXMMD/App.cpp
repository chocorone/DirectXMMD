#include "App.h"

LRESULT WindowProcedure(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	switch (msg)
	{
		//�E�B���h�E���j�����ꂽ��
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	//�K��̏������s��
	return DefWindowProc(hWnd, msg, wparam, lparam);
}

void MainLoop() 
{
	MSG msg = {};
	while (true) 
	{
		//�E�B���h�E���I�����Ă��Ȃ���Ή�
		if (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}	
	}
}

void StartApp(const TCHAR* appName)
{
	//�E�B���h�E�N���X�̐���
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//�R�[���o�b�N�̓o�^
	w.lpszClassName = appName;
	w.hInstance = GetModuleHandle(nullptr);//�n���h���̎擾
	w.hIcon = LoadIcon(w.hInstance, MAKEINTRESOURCE(101));
	//w.hIconSm = LoadIcon(w.hInstance, MAKEINTRESOURCE(101));
	RegisterClassEx(&w);//�E�B���h�E�N���X�̎w���OS�ɓ`����

	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
	//�E�B���h�E�T�C�Y�����߂�
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(w.lpszClassName,
		appName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,//�E�B���h�E��
		wrc.bottom - wrc.top,//�E�B���h�E����
		nullptr,//�e�E�B���h�E
		nullptr,//���j���[�n���h��
		w.hInstance,//�Ăяo���A�v���P�[�V����
		nullptr);//�ǉ��p�����[�^�[

	ShowWindow(hwnd, SW_SHOW);

	UnregisterClass(w.lpszClassName, w.hInstance);
	MainLoop();
}
