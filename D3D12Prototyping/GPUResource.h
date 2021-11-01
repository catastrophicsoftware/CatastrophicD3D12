#pragma once
#include "pch.h"
#include "DirectXHelpers.h"
#include "PlatformHelpers.h"

enum CENGINE_GPU_RESOURCE_TYPE
{
	Undefined = 0,
	Buffer = 1,
	Texture = 2,
};

struct InflightResourceRegion
{
	uint64 fenceValue;
	union ResourceRange
	{
		CD3DX12_RANGE bufferRange;
		CD3DX12_BOX textureBox;
	};
};

namespace DX
{
	class DeviceResources;
}

class IGPUResource
{
public:
	IGPUResource();
	IGPUResource(uint64 id,DX::DeviceResources* Engine);
	virtual ~IGPUResource() = 0;

	uint64* Map();
	void Unmap();

	virtual void Destroy() = 0;

	uint64 Id() const;
	D3D12_RESOURCE_STATES GetState() const;
	D3D12_RESOURCE_DESC GetDescription() const;
	CENGINE_GPU_RESOURCE_TYPE GetType() const;
protected:
	ID3D12Device* GPU;
	ID3D12Resource* Handle;
	DX::DeviceResources* Engine;

	uint64 id;

	uint32 cpuAccessible : 1;
	uint32 mapped : 1;
	uint64* pMappedMemory;

	D3D12_RESOURCE_DESC desc;
	D3D12_RESOURCE_STATES state;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	CENGINE_GPU_RESOURCE_TYPE resourceType;
};