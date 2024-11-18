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
	
	D3D12_DESCRIPTOR_HEAP_DESC hp;
	hp.NumDescriptors = GetFrameCount();
	hp.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hp.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	hp.NodeMask = 0;

	result = Context::get().getDevice()->CreateDescriptorHeap(&hp, IID_PPV_ARGS(_rtvHeap.GetAddressOf()));
	assert_if_SUCCEEDED(result, "Error on create the descriptor heap");

	auto firstHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	UINT handleIncrement = Context::get().getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (int i = 0; i < GetFrameCount(); i++)
	{
		_rtvHandles[i] = firstHandle;
		_rtvHandles[i].ptr += handleIncrement * i;
	}

	getBuffers();

    return true;
}

void Window::shutdown()
{

	releaseBuffers();

	_rtvHeap->Release();
	_rtvHeap.Detach();

	// TODO: Exit fullscreen before release 
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

		releaseBuffers();

		HRESULT result = _swapChain->ResizeBuffers(GetFrameCount(), _width, _height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		assert_if_SUCCEEDED(result, "Error on resize the buffers");
		
		getBuffers();
	}
	_resize = false;
}

void Window::SetFullscreen(bool enabled)
{

	if (enabled)
	{

		// Borderless fullscreen:
		SetWindowLongW(_windowHandle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowLongW(_windowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW);

		// To use dedicated fullscreen (First we need to resize the swapchain to the current monitor size to fit the monitor):
		// _swapChain->SetFullscreenState(true, Context::get().getOutput());
	}
	else
	{
		// Borderless fullscreen:
		SetWindowLongW(_windowHandle, GWL_STYLE, WINDOW_STYLES);
		SetWindowLongW(_windowHandle, GWL_EXSTYLE, WINDOW_EX_STYLES);
		
		// To use dedicated fullscreen (First we need to resize the swapchain to the current monitor size to fit the monitor):
		// _swapChain->SetFullscreenState(false, Context::get().getOutput());
	}
	_isFullscreen = enabled;


	IDXGIOutput* output;
	_swapChain->GetContainingOutput(&output);
	DXGI_OUTPUT_DESC desc;
	output->GetDesc(&desc);
	SetWindowPos(
		_windowHandle,
		nullptr,
		desc.DesktopCoordinates.left,
		desc.DesktopCoordinates.top,
		desc.DesktopCoordinates.right - desc.DesktopCoordinates.left,
		desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top,
		SWP_NOZORDER);


}

void Window::beginFrame(ID3D12GraphicsCommandList7* commandList)
{
	_currentBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER b;
	b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	b.Transition.pResource = _buffers[_currentBufferIndex].Get();
	b.Transition.Subresource = 0;
	b.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	b.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	commandList->ResourceBarrier(1, &b);
	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(_rtvHandles[_currentBufferIndex], clearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &_rtvHandles[_currentBufferIndex], false, nullptr);
}

void Window::endFrame(ID3D12GraphicsCommandList7* commandList)
{
	D3D12_RESOURCE_BARRIER b;
	b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	b.Transition.pResource = _buffers[_currentBufferIndex].Get();
	b.Transition.Subresource = 0;
	b.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	b.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	commandList->ResourceBarrier(1, &b);
}

void Window::getBuffers()
{
	for (int i = 0; i < GetFrameCount(); i++)
	{
		_swapChain->GetBuffer(i, IID_PPV_ARGS(_buffers[i].GetAddressOf()));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		Context::get().getDevice()->CreateRenderTargetView(_buffers[i].Get(), &rtvDesc, _rtvHandles[i]);
	}

	
}

void Window::releaseBuffers()
{
	for (int i = 0; i < GetFrameCount(); i++)
	{
		_buffers[i]->Release();
		_buffers[i].Detach();
	}
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
