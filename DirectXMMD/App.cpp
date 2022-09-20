#include "App.h"
#include "Engine.h"

DirectX::TexMetadata metadata = {};
const DirectX::Image *img;

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

			// g_Engine->SampleRender(metadata, img);
			// g_Engine->RotatePolygon(0.1f);
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

void StartApp(const TCHAR *appName)
{
	HWND hwnd = InitWindow(appName);

	g_Engine = new RenderingEngine();
	if (!g_Engine->Init(hwnd))
	{
		return;
	}

	PMDHeader pmdheader = {};

	char signature[3] = {};
	auto fp = fopen("./Model/tokino2.pmd", "rb");

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(PMDHeader), 1, fp);

	constexpr size_t pmdvertex_size = 38;

	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);

	std::vector<unsigned char> vertics(vertNum * pmdvertex_size);

	fread(vertics.data(), vertics.size(), 1, fp);

	fclose(fp);

	OutputDebugStringA(pmdheader.model_name);
	OutputDebugFormatedString("\n")
	OutputDebugStringA(pmdheader.comment);
	OutputDebugFormatedString("\n")
	OutputDebugFormatedString("頂点数：%d\n", vertNum)

	DirectX::ScratchImage scratchImg = {};

	LRESULT res = LoadFromWICFile(L"./img/sampleTex.png", DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(res))
	{
		OutputDebugFormatedString("テクスチャの読み込みに失敗");
		return;
	}
	img = scratchImg.GetImage(0, 0, 0);

	g_Engine->SampleRender(metadata, img);
	OutputDebugString(TEXT("一回目描画\n"));

	MainLoop();
}
