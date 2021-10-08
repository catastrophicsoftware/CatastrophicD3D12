#pragma once
#include "pch.h"
#include "GPUCommandQueue.h"
#include "GPUCommandAllocator.h"

class StaticGeometryBuffer
{
public:
	StaticGeometryBuffer(ID3D12Device* pDevice, Direct3DQueue* pCopyQueue);
	~StaticGeometryBuffer();

	HRESULT Create(uint64 size);

	D3D12_GPU_VIRTUAL_ADDRESS WriteVertices(void* pVertexData, uint64 vertexStride, uint64 numVertices);
	D3D12_GPU_VIRTUAL_ADDRESS WriteIndices(void* pIndexData, uint64 indexCount);

	ID3D12Resource* GetVertexBuffer() const;
	ID3D12Resource* GetIndexBuffer() const;

	void Commit();
	void Reset();
private:
	ID3D12Device* GPU;
	Direct3DQueue* CopyQueue;
	Direct3DQueue* GraphicsQueue;
	GPUCommandAllocator* CopyCmdAlloc;
	GPUCommandAllocator* DctCmdAlloc;

	D3D12_GPU_VIRTUAL_ADDRESS VertexGPUBaseAddr;
	D3D12_GPU_VIRTUAL_ADDRESS IndexGPUBaseAddr;

	ID3D12Resource* StaticVertexBuffer;
	ID3D12Resource* StagingVertexBuffer;

	ID3D12Resource* StaticIndexBuffer;
	ID3D12Resource* StagingIndexBuffer;

	void* pIndexStagingMemory;
	void* pVertexStagingMemory;

	uint64 vertexIndex;
	uint64 indexIndex;
	bool locked;
};