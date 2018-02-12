#pragma once

#include <cstdint>
#include <Windows.h>
#include <d3d11.h>

class Application
{
public:
	int32_t Init();

	int32_t Run();

public:

	virtual int32_t OnResize();

	virtual int32_t OnInit() { return 0; }

	virtual int32_t OnRelease() { return 0; }

	virtual int32_t OnUpdate() { return 0; }

	virtual void OnMouseDown(int button) { }

	virtual void OnMouseUp(int button) { }

	virtual void OnMouseMove(int x, int y) { }

	virtual void OnMouseWheel(int delta) {}

protected:
	bool						running;
	HINSTANCE					hInstance;
	HWND						hWnd;

	uint32_t					bufferWidth;
	uint32_t					bufferHeight;

	uint64_t					timeCounterFreq;
	uint64_t					startTime;
	uint64_t					lastFrameTime;
	uint64_t					currentTime;

	float						deltaTime;

	IDXGISwapChain*				swapChain;
	ID3D11Device*				device;
	ID3D11DeviceContext*		context;

	ID3D11RenderTargetView*		rtv;
	ID3D11DepthStencilView*		dsv;
};