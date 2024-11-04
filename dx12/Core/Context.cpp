#include "Context.h"

#include "custom_assert.h"

bool Context::init()
{


	HRESULT result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
	assert_if_not_SUCCEEDED(result, "Cant create DXGIFactory");

	ComPtr<IDXGIAdapter1> tempAdapter;
	result = _dxgiFactory->EnumAdapters1(1, tempAdapter.GetAddressOf());
	assert_if_not_SUCCEEDED(result, "Cant create DXGIAdapter");

	result = tempAdapter->QueryInterface(IID_PPV_ARGS(&_adapter));
	assert_if_not_SUCCEEDED(result, "Cant convert to IDXGIAdapter4");
	tempAdapter->Release();
	tempAdapter.Detach();

	DXGI_ADAPTER_DESC3 ad;
	_adapter->GetDesc3(&ad);

	result = D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));
	assert_if_not_SUCCEEDED(result, "Cant create Device");

	D3D12_COMMAND_QUEUE_DESC qd = {};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	qd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	qd.NodeMask = 0;
	qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	result = _device->CreateCommandQueue(&qd, IID_PPV_ARGS(&_commandQueue));
	assert_if_not_SUCCEEDED(result, "Cant create a CommandQueue");

	result = _device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	assert_if_not_SUCCEEDED(result, "Cant create a Fence");

	return true;
}

void Context::shutdown()
{

	_dxgiFactory->Release();
	_dxgiFactory.Detach();

	_adapter->Release();
	_adapter.Detach();

	_device->Release();
	_device.Detach();

	_commandQueue->Release();
	_commandQueue.Detach();

	_fence->Release();
	_fence.Detach();

}
