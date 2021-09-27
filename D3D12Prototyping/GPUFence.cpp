#include "pch.h"
#include "GPUFence.h"

GPUFence::GPUFence(ID3D12Device* pGPU, uint64 initialValue)
{
	lastCompletedFenceValue = initialValue;
	nextFenceValue = lastCompletedFenceValue + 1;

	GPU = pGPU;
	if (FAILED(GPU->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence))))
	{
		throw new std::runtime_error("failed to create fence");
	}
}

GPUFence::~GPUFence()
{
}

uint64 GPUFence::Signal(ID3D12CommandQueue* pQueue)
{
	uint64 signalVal = ++lastCompletedFenceValue;
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

uint64 GPUFence::GetNextValue() const
{
	return lastCompletedFenceValue + 1;
}

uint64 GPUFence::PollCurrentValue()
{
	lastCompletedFenceValue = std::max<uint64>(lastCompletedFenceValue, Fence->GetCompletedValue());

	return lastCompletedFenceValue;
}

bool GPUFence::IsComplete(uint64 fenceValue)
{
	if (fenceValue > lastCompletedFenceValue)
	{
		PollCurrentValue();
	}

	return fenceValue <= lastCompletedFenceValue;
}

HANDLE GPUFence::CreateFenceEvent()
{
	return CreateEvent(nullptr, false, false, nullptr);
}