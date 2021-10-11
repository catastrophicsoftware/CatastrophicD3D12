#pragma once
#include <d3d12.h>
#include "pch.h"
#include <stack>
#include "GPUCommandQueue.h"


class LinearConstantBuffer
{
public:
	LinearConstantBuffer();
	LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB, ID3D12Heap* targetHeap, uint64 heapOffset);
	LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB);
	~LinearConstantBuffer();

	void RegisterFence(InflightGPUWork workHandle);

	D3D12_GPU_VIRTUAL_ADDRESS Write(void* pData, uint64 dataSize);
	void Reset();
	void Destroy();

	uint64 GetConsumedMemory() const;
private:
	ID3D12Device* GPU;

	void Initialize(uint64 size,ID3D12Heap* pTargetHeap, uint64 heapOffset);
	void Initialize(uint64 size);
	uint8_t* pBaseGPUMem;
	uint8_t* pWritePtr;

	uint64 writeIndex;
	void Advance(uint64 lastDataSize);

	ID3D12Resource* buffer;
	D3D12_GPU_VIRTUAL_ADDRESS BaseAddress;

	InflightGPUWork fence;

	const size_t minConstantBufferAlignment = 256;
};