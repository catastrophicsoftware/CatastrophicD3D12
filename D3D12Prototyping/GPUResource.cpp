#include "pch.h"
#include "GPUResource.h"
#include "DeviceResources.h"

using namespace DirectX;

IGPUResource::IGPUResource(uint64 id, DX::DeviceResources* Engine) : GPU(nullptr),
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
    assert(Engine != nullptr);
    this->Engine = Engine;
    this->id = id;
    GPU = Engine->GetD3DDevice();

    //10-15-2021 -- maybe just self-reigster with the engine at this point
    //rather than creating all objects through the engine
    

    //TODO: Add a GetNextID() function to engine so resource ID's can be properly tracked
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