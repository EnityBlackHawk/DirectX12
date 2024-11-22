#include <iostream>

#include "basic_include.h"
#include "Core/DebugLayer.h"
#include "Core/Context.h"
#include "Core/Window.h"
#include "Core/custom_assert.h"
int main() {

	DebugLayer::get().init();

	if (Context::get().init() && Window::get().init()) {

		const char* hello = "Hello, World!";

		ComPtr<ID3D12Resource2> uploadbuffer, vertexBuffer;

		D3D12_HEAP_PROPERTIES hp = {};
		hp.Type = D3D12_HEAP_TYPE_UPLOAD; // CPU -> GPU
		hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hp.CreationNodeMask = 0;
		hp.VisibleNodeMask = 0;

		D3D12_HEAP_PROPERTIES hp_def = {};
		hp_def.Type = D3D12_HEAP_TYPE_DEFAULT; // CPU -> GPU
		hp_def.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hp_def.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hp_def.CreationNodeMask = 0;
		hp_def.VisibleNodeMask = 0;
		
		D3D12_RESOURCE_DESC rd = {};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rd.Width = 1024; // Size of upload buffer
		rd.Height = 1;
		rd.DepthOrArraySize = 1;
		rd.MipLevels = 1;
		rd.Format = DXGI_FORMAT_UNKNOWN;
		rd.SampleDesc.Count = 1;
		rd.SampleDesc.Quality = 0;
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // In order
		rd.Flags = D3D12_RESOURCE_FLAG_NONE;

		HRESULT result = Context::get().getDevice()->CreateCommittedResource(
			&hp,
			D3D12_HEAP_FLAG_NONE,
			&rd,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(uploadbuffer.GetAddressOf())
		);
		assert_if_SUCCEEDED(result, "Error on create the buffer");

		result = Context::get().getDevice()->CreateCommittedResource(
			&hp_def,
			D3D12_HEAP_FLAG_NONE,
			&rd,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(vertexBuffer.GetAddressOf())
		);
		assert_if_SUCCEEDED(result, "Error on create the vertexbuffer");

		// Copy data to the CPU Resource
		void* uploadBufferData;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1023;

		uploadbuffer->Map(0, &uploadRange, &uploadBufferData);
		memcpy(uploadBufferData, hello, strlen(hello) + 1);
		uploadbuffer->Unmap(0, &uploadRange);
		
		// CPU Resource -> GPU Resource
		auto* _cmdList = Context::get().prepareCommandList();
		_cmdList->CopyBufferRegion(vertexBuffer.Get(), 0, uploadbuffer.Get(), 0, 1024);
		Context::Context::get().executeCommandList();
		

		while (!Window::get().isClosed()) {
			// Process messages
			Window::get().update();
			if (Window::get().isResized())
			{
				Context::get().flush(Window::GetFrameCount());
				Window::get().resize();
			}
			// Begin draw
			auto* cmdList = Context::get().prepareCommandList();
			Window::get().beginFrame(cmdList);

			Window::get().endFrame(cmdList);
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