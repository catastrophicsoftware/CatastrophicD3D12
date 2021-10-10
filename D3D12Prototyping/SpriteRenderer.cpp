#include "pch.h"
#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(ID3D12Device* pGPU)
{
}

SpriteRenderer::~SpriteRenderer()
{
}

void SpriteRenderer::Initialize(uint32 backBufferCount)
{
	numBackbuffers = backBufferCount;

	InitializeGPUMemory();
}

void SpriteRenderer::BeginRenderPass(uint32 frameIndex)
{
}

void SpriteRenderer::EndRenderPass()
{
}

void SpriteRenderer::InitializeGPUMemory()
{
	PerFrameMemory = new LinearConstantBuffer[numBackbuffers];

	for (int i = 0; i < numBackbuffers; ++i)
	{
		PerFrameMemory = new LinearConstantBuffer(GPU, 16); //16MB of per-frame dynamic data
	}
}

void SpriteRenderer::InitializePipelineState()
{
	CD3DX12_ROOT_PARAMETER1 rootParameters[3];

	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	
	CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
}