#include "Window.h"

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

    return true;
}

void Window::shutdown()
{
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

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CLOSE:
        get()._closed = true;
		PostQuitMessage(0);
		break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
