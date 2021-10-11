#pragma once
#include "pch.h"

enum CENGINE_BUFFER_FLAGS
{
	BUFFER_FLAG_NONE = 1 << 0,
	BUFFER_FLAG_LIFETIME_MAP = 1 << 1,
};
DEFINE_ENUM_FLAG_OPERATORS(CENGINE_BUFFER_FLAGS)

class GPUBuffer
{
public:
	GPUBuffer();
	GPUBuffer(ID3D12Device* pGPU);
	~GPUBuffer();

	void Create(uint64 sizeInBytes, D3D12_RESOURCE_STATES initialState,CENGINE_BUFFER_FLAGS bufferFlags= BUFFER_FLAG_NONE, bool cpuAccessible=false);
	void Create(uint64 sizeInBytes, D3D12_RESOURCE_STATES initialState, CENGINE_BUFFER_FLAGS bufferFlags, bool cpuAccessible, ID3D12Heap* pTargetHeap, uint64 heapOffset);

	void StateTransition(ID3D12GraphicsCommandList* pCMD, D3D12_RESOURCE_STATES newState);

	void GPUCopyFromBuffer(ID3D12GraphicsCommandList* pCMD, uint64 offset, ID3D12Resource* pSourceBuffer, uint64 sourceOffset, uint64 numBytes, bool preserveState=true);
	void GPUCopyFromTexture(ID3D12GraphicsCommandList* pCMD);

	uint64* Map();
	void Unmap();

	bool Created() const;
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress();

	ID3D12Resource* Handle() const;

	void WriteElementAtIndex(uint64 elementSize, uint64 index, void* pData);
	D3D12_GPU_VIRTUAL_ADDRESS GetElementAddress(uint64 elementSize, uint64 index);

	void Release();
private:
	ID3D12Device* GPU;
	ID3D12Resource* buffer;
	uint64* pGPUMemory;

	CENGINE_BUFFER_FLAGS flags;
	D3D12_RESOURCE_STATES state;
	bool created;
	bool cpuAccessible;
	bool mapped;
	uint64 size;
	void createGPUBuffer(uint64 sizeInBytes,D3D12_RESOURCE_STATES initialState,bool cpuAccessible,CENGINE_BUFFER_FLAGS flags=BUFFER_FLAG_NONE, ID3D12Heap* pTargetHeap=nullptr, uint64 heapOffset=0);

	D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;
};