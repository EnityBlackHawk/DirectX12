#include <iostream>

#include "basic_include.h"
#include "Core/DebugLayer.h"
#include "Core/Context.h"
#include "Core/Window.h"
int main() {

	DebugLayer::get().init();

	if (Context::get().init() && Window::get().init()) {
		OutputDebugString(L"Device created");

		while (!Window::get().isClosed()) {
			Window::get().update();
			if (Window::get().isResized())
			{
				Context::get().flush(Window::GetFrameCount());
				Window::get().resize();
			}
			auto* cmdList = Context::get().prepareCommandList();

			Context::get().executeCommandList();
			Window::get().present();
		}
		// Flush
		Context::get().flush(Window::GetFrameCount());


		Window::get().shutdown();
		Context::get().shutdown();
	}

	
	DebugLayer::get().shutdown();
}