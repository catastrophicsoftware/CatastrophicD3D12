#pragma once
#include <d3d12.h>
#include "pch.h"



class LinearConstantBuffer
{
public:
	LinearConstantBuffer();
	LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB, ID3D12Heap* targetHeap, uint64 heapOffset);
	LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB);
	~LinearConstantBuffer();

	D3D12_GPU_VIRTUAL_ADDRESS Write(void* pData, uint64 dataSize);
	void Reset();
	void Reset(uint64 writeIndex); //reset the write index partially
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

	const size_t minConstantBufferAlignment = 256;
};