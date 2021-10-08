#pragma once
#include "pch.h"
#include "GPUDescriptorHeap.h"


class Texture2D
{
public:
	Texture2D(ID3D12Device* pGPU);
	~Texture2D();

	HRESULT Create(uint32 width, uint32 height, DXGI_FORMAT format);

	void Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData);
	void Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData, D3D12_RESOURCE_STATES newState);

	D3D12_RESOURCE_STATES GetResourceState() const;

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetUAV() const;

	void* GetGPUStagingMemory();

	void CreateSRV(GPUDescriptorHeap* pDescriptorHeap);
	void CreateUAV(GPUDescriptorHeap* pDescriptorHeap);
private:
	ID3D12Device* GPU;

	ID3D12Resource*       texture;
	D3D12_RESOURCE_STATES state;

	GPUDescriptorHandle   srv;
	bool hasSRV;

	GPUDescriptorHandle uav;
	bool hasUAV;

	ID3D12Resource* stagingBuffer;
	void* stagingGPUMem;

	uint64 size;
	uint32 width;
	uint32 height;
	DXGI_FORMAT format;
};