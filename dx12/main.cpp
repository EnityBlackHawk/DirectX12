#include <iostream>

#include "basic_include.h"
#include "Core/DebugLayer.h"
#include "Core/Context.h"
#include "Core/Window.h"
#include "Core/custom_assert.h"
#include "Core/Shader.h"
int main() {

	DebugLayer::get().init();

	if (Context::get().init() && Window::get().init()) {


		ComPtr<ID3D12Resource2> uploadbuffer, vertexBuffer;

		struct Vertices {
			float x, y;
		};

		Vertices vertices[] = {
			{-1.f, -1.f},
			{0.f,   1.f},
			{1.f,  -1.f}
		};
		D3D12_INPUT_ELEMENT_DESC vertexLayout[1] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		
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
		memcpy(uploadBufferData, vertices, sizeof(vertices));
		uploadbuffer->Unmap(0, &uploadRange);
		
		// CPU Resource -> GPU Resource
		auto* _cmdList = Context::get().prepareCommandList();
		_cmdList->CopyBufferRegion(vertexBuffer.Get(), 0, uploadbuffer.Get(), 0, 1024);
		Context::Context::get().executeCommandList();

		// Shaders
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		// PIPELINE STATE
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout.NumElements = _countof(vertexLayout);
		psoDesc.InputLayout.pInputElementDescs = vertexLayout;
		psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		psoDesc.VS.pShaderBytecode = vertexShader.data();
		psoDesc.VS.BytecodeLength = vertexShader.size();
		// TODO Resterizer
		psoDesc.PS.BytecodeLength = pixelShader.size();
		psoDesc.PS.pShaderBytecode = pixelShader.data();
		// TODO Output merger

		D3D12_VERTEX_BUFFER_VIEW vbv = {};
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(Vertices) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertices);
		
		

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
			
			// Draw
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);


			Window::get().endFrame(cmdList);
			// End draw and present
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