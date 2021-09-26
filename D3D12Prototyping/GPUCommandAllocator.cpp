#include "pch.h"
#include "GPUCommandAllocator.h"

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
    Allocator->Release();
    listsAllocated = 0;
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
