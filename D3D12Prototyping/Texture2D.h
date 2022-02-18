#pragma once
#include "pch.h"
#include "GPUDescriptorHeap.h"
#include <d3d12.h>

class Game;

class Texture2D
{
public:
	Texture2D(Game* pEngine);
	~Texture2D();

	HRESULT Create(uint32 width, uint32 height, DXGI_FORMAT format);
	HRESULT LoadFromDDS(std::wstring fileName, ID3D12CommandQueue* pQueue);

	void Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData);
	void Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData, D3D12_RESOURCE_STATES newState);

	D3D12_RESOURCE_STATES GetResourceState() const;

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetUAV() const;

	void* GetGPUStagingMemory();

	void CreateSRV(GPUDescriptorHeap* pDescriptorHeap);
	void CreateUAV(GPUDescriptorHeap* pDescriptorHeap);
private:
	Game* Engine;
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