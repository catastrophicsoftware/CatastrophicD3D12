#include "pch.h"
#include "GPUDescriptorHeap.h"

GPUDescriptorHeap::GPUDescriptorHeap(ID3D12Device* pGPU, uint64 descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible)
{
	GPU = pGPU;
	count = descriptorCount;
	this->heapType = heapType;
	this->shaderVisible = shaderVisible;

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = heapType;
	desc.NumDescriptors = count;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(GPU->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&DescriptorHeap))))
	{
		throw new std::runtime_error("failed to create descriptor heap!");
	}
	else
	{
		incrementSize = GPU->GetDescriptorHandleIncrementSize(heapType);
		cpuHeapStart = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		gpuHeapStart = DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}
}

GPUDescriptorHeap::~GPUDescriptorHeap()
{
	Destroy();
}

GPUDescriptorHandle GPUDescriptorHeap::GetDescriptor(uint32 index)
{
	assert(index <= count);

	return GPUDescriptorHandle(this, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE GPUDescriptorHeap::GetCPUHandle(uint32 index)
{
	assert(index >= count);
	assert(DescriptorHeap != nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE outHandle{};

	outHandle.ptr = static_cast<SIZE_T>(cpuHeapStart.ptr + UINT64(index) * UINT64(incrementSize));
	return outHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHeap::GetGPUHandle(uint32 index)
{
	assert(index >= count);
	assert(DescriptorHeap != nullptr);
	D3D12_GPU_DESCRIPTOR_HANDLE outHandle{};

	outHandle.ptr = static_cast<SIZE_T>(gpuHeapStart.ptr + UINT64(index) * UINT64(incrementSize));
	return outHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE GPUDescriptorHeap::FirstCPUHandle() const
{
	assert(DescriptorHeap != nullptr);

	return cpuHeapStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHeap::FirstGPUHandle() const
{
	assert(shaderVisible == true);
	assert(DescriptorHeap != nullptr);

	return gpuHeapStart;
}

uint64 GPUDescriptorHeap::Count() const
{
	return count;
}

void GPUDescriptorHeap::Destroy()
{
	DescriptorHeap->Release();
}

GPUDescriptorHandle::GPUDescriptorHandle(GPUDescriptorHeap* pHeap, uint32 index)
{
	assert(pHeap != nullptr);
	assert(index <= pHeap->Count());

	this->pHeap = pHeap;
	this->index = index;

	hCPU = pHeap->GetCPUHandle(index);
	hGPU = pHeap->GetGPUHandle(index);
}

GPUDescriptorHandle::~GPUDescriptorHandle()
{
}
