#include <iostream>

#include "basic_include.h"
#include "Debug/DebugLayer.h"
#include "Core/Context.h"
int main() {

	DebugLayer::get().init();

	if (Context::get().init()) {
		OutputDebugString(L"Device created");

		Context::get().shutdown();
	}

	
	DebugLayer::get().shutdown();
}