#include <Windows.h>
#include <debugapi.h>
#include "App.h"

int WINAPI WinMain(_In_ HINSTANCE hInsatance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	StartApp(TEXT("DirectX12てすと"));
	return 0;
}
