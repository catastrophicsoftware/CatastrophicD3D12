#include "pch.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(uint64 sizeInBytes, bool immutable, uint64 id, DX::DeviceResources* pEngine) : IGPUResource(id,pEngine)
{
	assert(sizeInBytes > 0);
	assert(pEngine != nullptr);
}

ConstantBuffer::~ConstantBuffer()
{
}