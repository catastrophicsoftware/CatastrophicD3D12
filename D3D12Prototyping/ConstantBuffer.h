#pragma once
#include "GPUResource.h"


class ConstantBuffer : public IGPUResource
{
public:
	ConstantBuffer(uint64 sizeInBytes, bool immutable, uint64 id, DX::DeviceResources* pEngine);
	virtual ~ConstantBuffer() override;

private:
	bool immutable; //if immutable, buffer will be placed in dedicated memory


};