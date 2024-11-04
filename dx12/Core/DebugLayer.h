#pragma once


#include "../basic_include.h"

using Microsoft::WRL::ComPtr;

class DebugLayer {

public:
	DebugLayer(const DebugLayer&) = delete;
	DebugLayer(DebugLayer&&) = delete;
	~DebugLayer();

	inline static DebugLayer& get() {
		static DebugLayer instance;
		return instance;
	}

	bool init();
	void shutdown();

private:
	DebugLayer() = default;
#ifdef _DEBUG
	ComPtr<ID3D12Debug6> _d3d12Debug;
	ComPtr<IDXGIDebug1> _dxgiDebug;
#endif // _DEBUG



};