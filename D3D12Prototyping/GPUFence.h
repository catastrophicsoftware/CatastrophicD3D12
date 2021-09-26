#pragma once
#include "pch.h"

class GPUFence
{
public:
	GPUFence(ID3D12Device* pGPU);
	~GPUFence();

	uint64 Signal(ID3D12CommandQueue* pQueue);
	void CPUWaitForSignal(uint64 fenceValue);
	void FlushGPUQueue(ID3D12CommandQueue* pQueue);

	ID3D12Fence* Handle() const;
	uint64 LastCompletedValue() const;
private:
	ID3D12Device* GPU;
	ID3D12Fence* Fence;

	uint64 value;

	HANDLE CreateFenceEvent();
};