#pragma once
#include "pch.h"

class GPUQueue;

struct InflightCommandList
{
	ID3D12CommandList* CMD;
	GPUQueue* pGPUQueue;
	uint64 fenceValue;

	InflightCommandList();
	InflightCommandList(ID3D12CommandList* pCMDList, GPUQueue* pGPUQueue, uint64 fenceValue);

	void CPUWait();
	bool IsComplete() const;
};

class GPUCommandAllocator
{
public:
	GPUCommandAllocator(ID3D12Device* pGPU, D3D12_COMMAND_LIST_TYPE Type);
	GPUCommandAllocator();

	ID3D12GraphicsCommandList* GetCommandList();

	void Reset();
	int NumAllocations();
	void Destroy();
private:
	ID3D12Device* GPU;
	ID3D12CommandAllocator* Allocator;
	D3D12_COMMAND_LIST_TYPE Type;

	int listsAllocated;
};