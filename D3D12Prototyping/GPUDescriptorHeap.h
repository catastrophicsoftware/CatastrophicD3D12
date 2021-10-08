#pragma once
#include "pch.h"

class GPUDescriptorHeap;


struct GPUDescriptorHandle
{
	GPUDescriptorHeap* pHeap;
	uint32 index;

	D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE hGPU;

	GPUDescriptorHandle(GPUDescriptorHeap* pHeap, uint32 index);
	~GPUDescriptorHandle();
};

class GPUDescriptorHeap
{
public:
	GPUDescriptorHeap(ID3D12Device* pGPU, uint64 descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType,bool shaderVisible=true);
	~GPUDescriptorHeap();

	GPUDescriptorHandle GetDescriptor(uint32 index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32 index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32 index);

	D3D12_CPU_DESCRIPTOR_HANDLE FirstCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUHandle() const;

	uint64 Count() const;
private:
	ID3D12Device* GPU;
	ID3D12DescriptorHeap* DescriptorHeap;

	uint64 count;
	bool shaderVisible;
	D3D12_DESCRIPTOR_HEAP_TYPE heapType;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;

	uint32 incrementSize;
};