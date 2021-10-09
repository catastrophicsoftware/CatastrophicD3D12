#pragma once
#include "pch.h"
#include "GPUCommandQueue.h"
#include "GPUCommandAllocator.h"
#include "LinearConstantBuffer.h"

class SpriteRenderer
{
public:
	SpriteRenderer(ID3D12Device* pGPU);
	~SpriteRenderer();

	void Initialize(uint32 backBufferCount);

	void BeginRenderPass(uint32 frameIndex);
	void EndRenderPass();

private:
	ID3D12Device* GPU;
	GPUQueue* GPUWorkQueue;
	GPUCommandAllocator* CommandAllocator;

	uint32 numBackbuffers;

	ID3D12PipelineState* SpritePipelineState;

	LinearConstantBuffer* PerFrameMemory;
	void InitializeGPUMemory();
	void InitializePipelineState();
};