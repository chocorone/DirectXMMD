#include "App.h"
PMDObject *obj;
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	//ウィンドウが破棄されたら
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		OutputDebugString(TEXT("終了\n"));
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wparam, lparam);
	}

	//規定の処理を行う
	return 0;
}

void MainLoop()
{
	MSG msg = {};
	while (true)
	{
		//ウィンドウが終了していなければ回す
		if (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			g_Engine->SampleRender(obj);
			g_Engine->RotatePolygon(0.05f);
		}
		else
		{
			// DispatchMessage(&msg);
			break;
		}
	}
}

HWND InitWindow(const TCHAR *appName)
{
	//ウィンドウクラスの生成
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバックの登録
	w.lpszClassName = appName;
	w.hInstance = GetModuleHandle(nullptr); //ハンドルの取得
	w.hIcon = LoadIcon(w.hInstance, MAKEINTRESOURCE(101));
	// w.hIconSm = LoadIcon(w.hInstance, MAKEINTRESOURCE(101));
	RegisterClassEx(&w); //ウィンドウクラスの指定をOSに伝える

	RECT wrc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	//ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(w.lpszClassName,
							 appName,
							 WS_OVERLAPPEDWINDOW,
							 CW_USEDEFAULT,
							 CW_USEDEFAULT,
							 wrc.right - wrc.left, //ウィンドウ幅
							 wrc.bottom - wrc.top, //ウィンドウ高さ
							 nullptr,			   //親ウィンドウ
							 nullptr,			   //メニューハンドル
							 w.hInstance,		   //呼び出しアプリケーション
							 nullptr);			   //追加パラメーター

	ShowWindow(hwnd, SW_SHOW);

	UnregisterClass(w.lpszClassName, w.hInstance);

	return hwnd;
}

void LoadPMD()
{
}

void StartApp(const TCHAR *appName)
{
	HWND hwnd = InitWindow(appName);

	g_Engine = new RenderingEngine();
	if (!g_Engine->Init(hwnd))
	{
		return;
	}

	obj = new PMDObject;
	obj->LoadData("./Model/tokino2.pmd");

	g_Engine->SampleRender(obj);
	OutputDebugString(TEXT("一回目描画\n"));

	MainLoop();
}
