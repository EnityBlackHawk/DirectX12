#include "Window.h"
#include "Context.h"
#include "custom_assert.h"

bool Window::init()
{
    // Window class

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = &Window::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"DefWindow";
	wc.style = CS_OWNDC;


	_windowClass = RegisterClassExW(&wc);
    
    assert(_windowClass && "Error on register the window class");

    _windowHandle = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
		(LPCWSTR)_windowClass,
		L"DX12",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr
	);

    assert(_windowHandle && "Error on create the window");

	DXGI_SWAP_CHAIN_DESC1 scd = {};
	scd.Width = 800;
	scd.Height = 600;
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.Stereo = false;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = GetFrameCount(); // = 2 | if vsync is enabled, it will be 3
	scd.Scaling = DXGI_SCALING_STRETCH;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd = {};
	//fsd.RefreshRate = ;
	//fsd.Scaling = DXGI_MODE_SCALING_CENTERED;
	//fsd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fsd.Windowed = true;

	ComPtr<IDXGISwapChain1> swapChain;
	HRESULT result = Context::get().getDxgiFactory()->CreateSwapChainForHwnd(
		Context::get().getCommandQueue().Get(),
		_windowHandle,
		&scd,
		&fsd,
		nullptr,
		swapChain.GetAddressOf()
	);
	
	assert_if_SUCCEEDED(result, "Error on create the swap chain");

	result = swapChain->QueryInterface(IID_PPV_ARGS(&_swapChain));
	assert_if_SUCCEEDED(result, "Error on convert to IDXGISwapChain4");

    return true;
}

void Window::shutdown()
{

	_swapChain->Release();
	_swapChain.Detach();

    if (_windowHandle) {
        DestroyWindow(_windowHandle);
    }

    if (_windowClass)
    {
		UnregisterClassW((LPCWSTR)_windowClass, GetModuleHandle(nullptr));
    }
}

void Window::update() const
{
	MSG msg = {};
	while (PeekMessageW(&msg, _windowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

}

void Window::present()
{
	_swapChain->Present(1, 0);
}

void Window::resize()
{
	if (_swapChain)
	{
		RECT rc;
		GetClientRect(_windowHandle, &rc);
		_width = rc.right - rc.left;
		_height = rc.bottom - rc.top;
		_swapChain->ResizeBuffers(GetFrameCount(), _width, _height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		
	}
	_resize = false;

}

void Window::SetFullscreen(bool enabled)
{
	if (enabled)
	{

		SetWindowLongW(_windowHandle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowLongW(_windowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		

		// _swapChain->SetFullscreenState(true, nullptr);
	}
	else
	{

		SetWindowLongW(_windowHandle, GWL_STYLE, WINDOW_STYLES);
		SetWindowLongW(_windowHandle, GWL_EXSTYLE, WINDOW_EX_STYLES);

		// _swapChain->SetFullscreenState(false, nullptr);
	}
	_isFullscreen = enabled;

	IDXGIOutput6* output = Context::get().getOutput();
	DXGI_OUTPUT_DESC1 desc;
	output->GetDesc1(&desc);
	SetWindowPos(
		_windowHandle,
		nullptr,
		desc.DesktopCoordinates.left,
		desc.DesktopCoordinates.top,
		desc.DesktopCoordinates.right - desc.DesktopCoordinates.left,
		desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top,
		SWP_NOZORDER);

}

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_F11)
		{
			get().SetFullscreen(!get()._isFullscreen);
		}
		break;
	case WM_SIZE:
		if (lParam && ( LOWORD(lParam) != get()._width || HIWORD(lParam) != get()._height )) {
			get()._resize = true;
		}
		break;

	case WM_CLOSE:
        get()._closed = true;
		PostQuitMessage(0);
		break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
