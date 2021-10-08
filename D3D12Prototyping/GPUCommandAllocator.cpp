#include "pch.h"
#include "GPUCommandAllocator.h"
#include "GPUCommandQueue.h"

GPUCommandAllocator::GPUCommandAllocator(ID3D12Device* pGPU, D3D12_COMMAND_LIST_TYPE Type)
{
    this->GPU = pGPU;
    this->Type = Type;
    listsAllocated = 0;
    if (FAILED(GPU->CreateCommandAllocator(Type, IID_PPV_ARGS(&Allocator))))
    {
        throw new std::runtime_error("Failed to create command allocator!");
    }
}

GPUCommandAllocator::GPUCommandAllocator()
{
    Destroy();
}

ID3D12GraphicsCommandList* GPUCommandAllocator::GetCommandList()
{
    ID3D12GraphicsCommandList* pCommandList = nullptr;
    if (FAILED(GPU->CreateCommandList(0, Type, Allocator, nullptr, IID_PPV_ARGS(&pCommandList))))
    {
        throw new std::runtime_error("failed to allocate command list!");
    }

    listsAllocated++;
    return pCommandList;
}

void GPUCommandAllocator::Reset()
{
    Allocator->Reset();
}

int GPUCommandAllocator::NumAllocations()
{
    return listsAllocated;
}

void GPUCommandAllocator::Destroy()
{
    Allocator->Release(); //currently no safety preventing this from being called on command lists that are being executed by the gpu
    listsAllocated = 0;
}

InflightCommandBuffer::InflightCommandBuffer()
{
}

InflightCommandBuffer::InflightCommandBuffer(ID3D12CommandList* pCMDList, Direct3DQueue* pGPUQueue, uint64 fenceValue)
{
    CMD = pCMDList;
    this->pGPUQueue = pGPUQueue;
    this->fenceValue = fenceValue;
}

void InflightCommandBuffer::CPUWait()
{
    //block current thread until gpu execution is complete
    pGPUQueue->WaitForFenceCPUBlocking(fenceValue);
}

bool InflightCommandBuffer::IsComplete() const
{
    return pGPUQueue->GetLastCompletedFence() >= fenceValue;
}