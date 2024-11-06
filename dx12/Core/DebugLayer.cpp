#include "DebugLayer.h"

DebugLayer::~DebugLayer()
{
#ifdef _DEBUG
	_d3d12Debug->Release();
	_d3d12Debug.Detach();
	_dxgiDebug->Release();
	_dxgiDebug.Detach();
#endif // _DEBUG

}

bool DebugLayer::init()
{
#ifdef _DEBUG
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&_d3d12Debug));
	if (SUCCEEDED(result)) {
		_d3d12Debug->EnableDebugLayer();

		result = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&_dxgiDebug));
		if (SUCCEEDED(result)) {
			_dxgiDebug->EnableLeakTrackingForThread();
			return true;
		}
	}
	return true;
#endif
	return false;
}

void DebugLayer::shutdown()
{
#ifdef _DEBUG

	if (_dxgiDebug)
	{
		OutputDebugStringW(L"DXGI Report: \n");
		_dxgiDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL,
			DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
		);
	}

	_dxgiDebug->DisableLeakTrackingForThread();
	_d3d12Debug->DisableDebugLayer();
#endif
}
