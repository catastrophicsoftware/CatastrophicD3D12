#include "pch.h"
#include "GPUResource.h"

GPUResource::GPUResource(ID3D12Device* pGPU)
{
}

GPUResource::~GPUResource()
{
}

uint64 GPUResource::Id() const
{
    return id;
}