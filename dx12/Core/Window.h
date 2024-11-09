#pragma once
#include "../basic_include.h"
#include "Singleton.h"

using Microsoft::WRL::ComPtr;

class Window : public Singleton<Window>
{
public:

	// Inherited via Singleton
	bool init() override;
	void shutdown() override;
	void update() const;
	void present();
	void resize();

	inline static bool isClosed() { return get()._closed; }
	inline static bool isResized() { return get()._resize; }

	static constexpr int GetFrameCount() { return 2; }

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	ComPtr<IDXGISwapChain4> _swapChain;

	bool _closed = false;
	bool _resize = false;
	ATOM _windowClass = 0;
	HWND _windowHandle = nullptr;
	int _width;
	int _height;
};