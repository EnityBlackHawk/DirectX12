#pragma once
#include "../basic_include.h"
#include "Singleton.h"

using Microsoft::WRL::ComPtr;

constexpr DWORD WINDOW_EX_STYLES = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
constexpr DWORD WINDOW_STYLES = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

class Window : public Singleton<Window>
{
public:

	// Inherited via Singleton
	bool init() override;
	void shutdown() override;
	void update() const;
	void present();
	void resize();
	void SetFullscreen(bool enabled);

	void beginFrame(ID3D12GraphicsCommandList7* commandList);
	void endFrame(ID3D12GraphicsCommandList7* commandList);

	inline static bool isClosed() { return get()._closed; }
	inline static bool isResized() { return get()._resize; }
	
	static constexpr int frameCount = 2;
	static constexpr int GetFrameCount() { return frameCount; }

private:

	void getBuffers();
	void releaseBuffers();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	


	ComPtr<IDXGISwapChain4> _swapChain;
	ComPtr<ID3D12Resource2> _buffers[frameCount];
	int _currentBufferIndex = 0;

	bool _closed = false;
	bool _resize = false;
	bool _isFullscreen = false;
	ATOM _windowClass = 0;
	HWND _windowHandle = nullptr;
	int _width;
	int _height;
};