#include "Context.h"

#include "custom_assert.h"

bool Context::init()
{


	HRESULT result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
	assert_if_SUCCEEDED(result, "Cant create DXGIFactory");

	ComPtr<IDXGIAdapter1> tempAdapter;
	result = _dxgiFactory->EnumAdapters1(1, tempAdapter.GetAddressOf());
	assert_if_SUCCEEDED(result, "Cant create DXGIAdapter");

	result = tempAdapter->QueryInterface(IID_PPV_ARGS(&_adapter));
	assert_if_SUCCEEDED(result, "Cant convert to IDXGIAdapter4");
	tempAdapter->Release();
	tempAdapter.Detach();

	ComPtr<IDXGIOutput> tmp_output;
	result = _adapter->EnumOutputs(0, tmp_output.GetAddressOf());
	assert_if_SUCCEEDED(result, "Cant create DXGIOutput");

	result = tmp_output->QueryInterface(IID_PPV_ARGS(&_output));
	assert_if_SUCCEEDED(result, "Cant convert to IDXGIOutput6");

	result = D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));
	assert_if_SUCCEEDED(result, "Cant create Device");

	D3D12_COMMAND_QUEUE_DESC qd = {};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	qd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	qd.NodeMask = 0;
	qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	result = _device->CreateCommandQueue(&qd, IID_PPV_ARGS(&_commandQueue));
	assert_if_SUCCEEDED(result, "Cant create a CommandQueue");

	result = _device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	assert_if_SUCCEEDED(result, "Cant create a Fence");

	// maybe create the event here

	_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	assert_if_SUCCEEDED(result, "Cant create a CommandAllocator");
	
	_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&_commandList));
	assert_if_SUCCEEDED(result, "Cant create a CommandList");
	


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

	_commandList->Release();
	_commandList.Detach();

	_commandAllocator->Release();
	_commandAllocator.Detach();

	_fence->Release();
	_fence.Detach();

}

void Context::signalAndWait()
{
	_fenceValue++;
	_commandQueue->Signal(_fence.Get(), _fenceValue);

	if (_fence->GetCompletedValue() < _fenceValue) {
		HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		_fence->SetEventOnCompletion(_fenceValue, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}

ID3D12GraphicsCommandList7* Context::prepareCommandList()
{
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator.Get(), nullptr);
	return _commandList.Get();
}

void Context::executeCommandList()
{
	_commandList->Close();
	ID3D12CommandList* lists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, lists);
	signalAndWait();
}
