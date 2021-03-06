#include "pch.h"
#include "GPUDescriptorHeap.h"
#include "Exceptions.h"

GPUDescriptorHeap::GPUDescriptorHeap(ID3D12Device* pGPU, uint64 descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible)
{
	GPU = pGPU;
	count = descriptorCount;
	this->heapType = heapType;
	this->shaderVisible = shaderVisible;
	index = 0;

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

		for (int i = 0; i < count; ++i)
		{
			auto unusedDescriptorHandle = GetDescriptor(i);
			UnusedHandles.push(unusedDescriptorHandle);
		}
	}
}

GPUDescriptorHeap::~GPUDescriptorHeap()
{
	Destroy();
}

GPUDescriptorHandle GPUDescriptorHeap::GetDescriptor(uint32 index)
{
	return GPUDescriptorHandle(this, index);
}

GPUDescriptorHandle GPUDescriptorHeap::GetUnusedDescriptor()
{
	if (!UnusedHandles.empty())
	{
		GPUDescriptorHandle unusedHandle = UnusedHandles.front();
		UnusedHandles.pop();
		index++;
		return unusedHandle;
	}
	else
		throw DescriptorPoolFullException("descriptor pool has no unused descriptors!");
}

D3D12_CPU_DESCRIPTOR_HANDLE GPUDescriptorHeap::GetCPUHandle(uint32 index)
{
	assert(index <= (count-1));
	assert(DescriptorHeap != nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE outHandle{};

	outHandle.ptr = static_cast<SIZE_T>(cpuHeapStart.ptr + UINT64(index) * UINT64(incrementSize));
	return outHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHeap::GetGPUHandle(uint32 index)
{
	assert(index <= (count - 1));
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

ID3D12DescriptorHeap* GPUDescriptorHeap::HeapHandle() const
{
	return DescriptorHeap;
}

uint64 GPUDescriptorHeap::Count() const
{
	return count;
}

uint32 GPUDescriptorHeap::UsedDescriptors() const
{
	return index;
}

bool GPUDescriptorHeap::IsFull() const
{
	return (index >= (count-1));
}

void GPUDescriptorHeap::Destroy()
{
	DescriptorHeap->Release();
}

D3D12_DESCRIPTOR_HEAP_TYPE GPUDescriptorHeap::GetHeapType() const
{
	return heapType;
}

GPUDescriptorHandle::GPUDescriptorHandle()
{
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
