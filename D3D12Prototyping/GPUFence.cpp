#include "pch.h"
#include "GPUFence.h"

GPUFence::GPUFence(ID3D12Device* pGPU)
{
	value = 0;
	GPU = pGPU;
	if (FAILED(GPU->CreateFence(value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence))))
	{
		throw new std::runtime_error("failed to create fence");
	}
}

GPUFence::~GPUFence()
{
}

uint64 GPUFence::Signal(ID3D12CommandQueue* pQueue)
{
	uint64 signalVal = ++value;
	pQueue->Signal(Fence, signalVal);
	return signalVal;
}

void GPUFence::CPUWaitForSignal(uint64 fenceValue)
{
	if (Fence->GetCompletedValue() > fenceValue) //already reached, return
		return;
	else
	{
		HANDLE completionEvent = CreateFenceEvent();
		Fence->SetEventOnCompletion(fenceValue, completionEvent);
		WaitForSingleObject(completionEvent, INFINITE);
	}
}

void GPUFence::FlushGPUQueue(ID3D12CommandQueue* pQueue)
{
	uint64 flushSignalValue = Signal(pQueue);
	CPUWaitForSignal(flushSignalValue);
}

ID3D12Fence* GPUFence::Handle() const
{
	return Fence;
}

uint64 GPUFence::LastCompletedValue() const
{
	return Fence->GetCompletedValue();
}

HANDLE GPUFence::CreateFenceEvent()
{
	return CreateEvent(nullptr, false, false, nullptr);
}