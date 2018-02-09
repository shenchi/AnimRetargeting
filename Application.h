#pragma once

#include <cstdint>
#include <Windows.h>
#include <d3d11.h>

class Application
{
private:
	bool						running;
	HINSTANCE					hInstance;
	HWND						hWnd;

	uint32_t					bufferWidth;
	uint32_t					bufferHeight;

	IDXGISwapChain*				swapChain;
	ID3D11Device*				device;
	ID3D11DeviceContext*		context;

	ID3D11RenderTargetView*		rtv;
	ID3D11DepthStencilView*		dsv;

public:
	int32_t Init();

	int32_t Run();

public:

	virtual int32_t OnResize();

	virtual int32_t OnInit() { return 0; }

	virtual int32_t OnRelease() { return 0; }
};