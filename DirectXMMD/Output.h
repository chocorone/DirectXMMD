#pragma once
#ifdef _DEBUG
#define OutputDebugFormatedString(str, ...)       \
	{                                             \
		TCHAR c[256];                             \
		swprintf(c, 256, TEXT(str), __VA_ARGS__); \
		OutputDebugString(c);                     \
	}
#else
#define MyOutputDebugString(str, ...) // ‹óŽÀ‘•
#endif