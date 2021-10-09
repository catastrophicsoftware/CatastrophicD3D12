#include "pch.h"
#include "SpriteRenderer.h"

void SpriteRenderer::Initialize(uint32 backBufferCount)
{
	numBackbuffers = backBufferCount;

	InitializeGPUMemory();
}

void SpriteRenderer::InitializeGPUMemory()
{
	PerFrameMemory = new LinearConstantBuffer[numBackbuffers];

	for (int i = 0; i < numBackbuffers; ++i)
	{
		PerFrameMemory = new LinearConstantBuffer(GPU, 16); //16MB of per-frame dynamic data
	}
}