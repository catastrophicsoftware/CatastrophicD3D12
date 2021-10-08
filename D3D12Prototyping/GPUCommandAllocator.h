#pragma once
#include "pch.h"

struct InflightCommandBuffer
{
	ID3D12GraphicsCommandList* CMD;
	UINT64 fenceValue;
};

class GPUCommandAllocator
{
public:
	GPUCommandAllocator(ID3D12Device* pGPU, D3D12_COMMAND_LIST_TYPE Type);
	GPUCommandAllocator();

	ID3D12GraphicsCommandList* GetCommandList();

	void Reset();

	int NumAllocations();
private:
	ID3D12Device* GPU;
	ID3D12CommandAllocator* Allocator;
	D3D12_COMMAND_LIST_TYPE Type;

	int listsAllocated;
};