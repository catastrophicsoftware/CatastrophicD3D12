#pragma once
#include "pch.h"

enum CENGINE_GPU_RESOURCE_TYPE
{
	BUFFER = 1,
	TEXTURE = 2,
};

struct InflightResourceRegion
{
	uint64 fenceValue;
};

class GPUResource
{
public:
	GPUResource(ID3D12Device* pGPU);
	~GPUResource();

	uint64 Id() const;
protected:
	uint64 id;

	uint32 cpuAccessible : 1;
	uint32 mapped : 1;
};