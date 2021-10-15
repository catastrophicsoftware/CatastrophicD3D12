#include "pch.h"
#include "GPUResource.h"
#include "DeviceResources.h"

using namespace DirectX;

IGPUResource::IGPUResource(uint64 id, DX::DeviceResources* pEngine, ID3D12Resource* pResourceHandle, bool cpuAccessible, D3D12_RESOURCE_STATES initialState, CENGINE_GPU_RESOURCE_TYPE type)
{
    assert(pEngine != nullptr && pResourceHandle != nullptr);
    Engine = pEngine;
    GPU = Engine->GetD3DDevice();
    Handle = pResourceHandle;
    this->cpuAccessible = cpuAccessible;
    state = initialState;
    desc = pResourceHandle->GetDesc();
    gpuAddress = Handle->GetGPUVirtualAddress();
    mapped = false;
    resourceType = type;
}

IGPUResource::IGPUResource() : GPU(nullptr),
    Handle(nullptr),
    id(0),
    cpuAccessible(false),
    mapped(false),
    state(D3D12_RESOURCE_STATE_COMMON),
    gpuAddress(0),
    resourceType(Undefined),
    pMappedMemory(nullptr),
    desc({})
{
}

IGPUResource::~IGPUResource()
{
}

uint64* IGPUResource::Map()
{
    assert(cpuAccessible);

    if (mapped)
        return pMappedMemory;
    else
    {
        CD3DX12_RANGE readRange{};
        void* pTempMap;
        ThrowIfFailed(Handle->Map(0, &readRange, &pTempMap));
        mapped = true;
        pMappedMemory = static_cast<uint64*>(pTempMap);
        return pMappedMemory;
    }
}

void IGPUResource::Unmap()
{
    assert(cpuAccessible && mapped);
    Handle->Unmap(0, nullptr);
    mapped = false;
}

uint64 IGPUResource::Id() const
{
    return id;
}

D3D12_RESOURCE_STATES IGPUResource::GetState() const
{
    return D3D12_RESOURCE_STATES();
}

D3D12_RESOURCE_DESC IGPUResource::GetDescription() const
{
    return desc;
}

CENGINE_GPU_RESOURCE_TYPE IGPUResource::GetType() const
{
    return resourceType;
}