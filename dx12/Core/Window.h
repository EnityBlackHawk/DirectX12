#pragma once
#include "../basic_include.h"
#include "Singleton.h"

class Window : public Singleton<Window>
{
public:

	// Inherited via Singleton
	bool init() override;
	void shutdown() override;
	void update() const;

	inline static bool isClosed() { return get()._closed; }

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool _closed = false;

	ATOM _windowClass = 0;
	HWND _windowHandle = nullptr;
};