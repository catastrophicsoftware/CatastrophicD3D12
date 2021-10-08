#pragma once
#include "pch.h"

class Texture2D
{
public:
	Texture2D(ID3D12Device* pGPU);
	~Texture2D();

	HRESULT Create(uint32 width, uint32 height, DXGI_FORMAT format);

	void Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData);
	void Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData, D3D12_RESOURCE_STATES newState);

	D3D12_RESOURCE_STATES GetResourceState() const;

	void* GetGPUStagingMemory();
private:
	ID3D12Device* GPU;

	void CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
	ID3D12Resource* texture;
	D3D12_RESOURCE_STATES state;

	ID3D12Resource* stagingBuffer;
	void* stagingGPUMem;

	uint64 size;
	uint32 width;
	uint32 height;
	DXGI_FORMAT format;
};