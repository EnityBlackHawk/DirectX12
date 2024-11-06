#pragma once
#include "../basic_include.h"

using Microsoft::WRL::ComPtr;

class Context
{
public:
	Context(const Context&) = delete;
	Context(Context&&) = delete;

	inline static Context& get() {
		static Context instance;
		return instance;
	}

	bool init();
	void shutdown();
	void signalAndWait();

	ID3D12GraphicsCommandList7* prepareCommandList();
	void executeCommandList();

private:
	Context() = default;
	
	ComPtr<IDXGIFactory7> _dxgiFactory;
	ComPtr<IDXGIAdapter4> _adapter;
	ComPtr<ID3D12Device10> _device;
	ComPtr<ID3D12CommandQueue> _commandQueue;
	ComPtr<ID3D12Fence1> _fence;
	ComPtr<ID3D12GraphicsCommandList7> _commandList;
	ComPtr<ID3D12CommandAllocator> _commandAllocator;
	UINT64 _fenceValue = 0;

	

};

