#include "Application.h"

#include <cassert>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace
{
	LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Application* app = nullptr;
		switch (msg)
		{
		case WM_SIZE:
			app = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if (app != nullptr)
			{
				app->OnResize();
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

int32_t Application::Init()
{
	hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX wcx;

	// Fill in the window class structure with parameters 
	// that describe the main window. 

	wcx.cbSize = sizeof(wcx);          // size of structure 
	wcx.style = CS_HREDRAW |
		CS_VREDRAW;                    // redraw if size changes 
	wcx.lpfnWndProc = MainWndProc;     // points to window procedure 
	wcx.cbClsExtra = 0;                // no extra class memory 
	wcx.cbWndExtra = 0;                // no extra window memory 
	wcx.hInstance = hInstance;         // handle to instance 
	wcx.hIcon = nullptr;
	wcx.hCursor = LoadCursor(NULL,
		IDC_ARROW);                    // predefined arrow 
	wcx.hbrBackground = (HBRUSH)GetStockObject(
		WHITE_BRUSH);                  // white background brush 
	wcx.lpszMenuName = L"MainMenu";    // name of menu resource 
	wcx.lpszClassName = L"MainWClass";  // name of window class 
	wcx.hIconSm = nullptr;

	// Register the window class. 

	if (!RegisterClassEx(&wcx))
		return __LINE__;

	// Create the main window. 

	hWnd = CreateWindow(
		L"MainWClass",        // name of window class 
		L"Sample",            // title-bar string 
		WS_OVERLAPPEDWINDOW, // top-level window 
		CW_USEDEFAULT,       // default horizontal position 
		CW_USEDEFAULT,       // default vertical position 
		CW_USEDEFAULT,       // default width 
		CW_USEDEFAULT,       // default height 
		(HWND)NULL,         // no owner window 
		(HMENU)NULL,        // use class menu 
		hInstance,           // handle to application instance 
		(LPVOID)NULL);      // no window-creation data 

	if (!hWnd)
		return __LINE__;

	// Show the window and send a WM_PAINT message to the window 
	// procedure. 

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	RECT rect = {};
	GetClientRect(hWnd, &rect);
	uint32_t bufferWidth = rect.right - rect.left;
	uint32_t bufferHeight = rect.bottom - rect.top;

	IDXGIAdapter1* adapter = nullptr;
	{
		IDXGIFactory1* factory;
		CreateDXGIFactory1(IID_PPV_ARGS(&factory));

		//factory->MakeWindowAssociation(hWnd,)
		UINT i = 0;
		while (SUCCEEDED(factory->EnumAdapters1(i++, &adapter)))
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (!(desc.Flags && DXGI_ADAPTER_FLAG_SOFTWARE))
				break;

			adapter->Release();
			adapter = nullptr;
		}

		factory->Release();
	}

	if (nullptr == adapter)
	{
		return __LINE__;
	}

	UINT dxFlags = 0;
#ifdef _DEBUG
	dxFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = bufferWidth;
	swapChainDesc.BufferDesc.Height = bufferHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		//D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		dxFlags,
		featureLevels, 2,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		nullptr,
		&context)))
	{
		return __LINE__;
	}

	adapter->Release();

	OnResize();

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	return 0;
}

int32_t Application::Run()
{
	MSG msg;

	OnInit();

	running = true;
	while (running)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				running = false;
			}
		}

		float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
		context->ClearRenderTargetView(rtv, clearColor);

		swapChain->Present(0, 0);
	}

	OnRelease();

	dsv->Release();
	rtv->Release();
	context->Release();
	device->Release();
	swapChain->Release();

	return msg.wParam;
}

int32_t Application::OnResize()
{
	RECT rect = {};
	GetClientRect(hWnd, &rect);
	uint32_t newBufferWidth = rect.right - rect.left;
	uint32_t newBufferHeight = rect.bottom - rect.top;

	if (newBufferWidth == bufferWidth && newBufferHeight == bufferHeight)
	{
		return 0;
	}

	bufferWidth = newBufferWidth;
	bufferHeight = newBufferHeight;

	if (nullptr != rtv)
	{
		rtv->Release();
		rtv = nullptr;
	}
	if (nullptr != dsv)
	{
		dsv->Release();
		dsv = nullptr;
	}
	if (FAILED(swapChain->ResizeBuffers(2, bufferWidth, bufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
	{
		return __LINE__;
	}

	ID3D11Texture2D* backBuffer;
	if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
	{
		return __LINE__;
	}

	if (FAILED(device->CreateRenderTargetView(backBuffer, nullptr, &rtv)))
	{
		return __LINE__;
	}

	backBuffer->Release();

	ID3D11Texture2D* depthBuffer;
	CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_D24_UNORM_S8_UINT, bufferWidth, bufferHeight,
		1, 0, D3D11_BIND_DEPTH_STENCIL);
	if (FAILED(device->CreateTexture2D(&desc, nullptr, &depthBuffer)))
	{
		return __LINE__;
	}
	if (FAILED(device->CreateDepthStencilView(depthBuffer, nullptr, &dsv)))
	{
		return __LINE__;
	}
	depthBuffer->Release();

	return 0;
}