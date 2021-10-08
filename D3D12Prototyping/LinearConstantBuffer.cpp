#include "pch.h"
#include "LinearConstantBuffer.h"

LinearConstantBuffer::LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB, ID3D12Heap* targetHeap, uint64 heapOffset)
{
	writeIndex = 0;
	this->GPU = GPU;
	Initialize(sizeInMB, targetHeap, heapOffset);
}

LinearConstantBuffer::~LinearConstantBuffer()
{
	Destroy();
}

D3D12_GPU_VIRTUAL_ADDRESS LinearConstantBuffer::Write(void* pData, uint64 dataSize)
{
	D3D12_GPU_VIRTUAL_ADDRESS writeOffset = BaseAddress + writeIndex;
	memcpy(pBaseGPUMem + writeIndex, pData, dataSize); //copy into gpu memory
	Advance(dataSize); //advance write pointers

	return writeOffset;
}

void LinearConstantBuffer::Reset()
{
	writeIndex = 0;
}

void LinearConstantBuffer::Destroy()
{
	buffer->Release();
	writeIndex = 0;
}

void LinearConstantBuffer::Initialize(uint64 size, ID3D12Heap* targetHeap, uint64 heapOffset)
{
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	if (FAILED(GPU->CreatePlacedResource(targetHeap, heapOffset, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))))
	{
		throw new std::runtime_error("failed to create dynamic constant buffer!");
	}
	else
	{
		BaseAddress = buffer->GetGPUVirtualAddress();
		CD3DX12_RANGE readRange(0, 0);
		buffer->Map(0, &readRange,(void**)&pBaseGPUMem);
	}
}

void LinearConstantBuffer::Advance(uint64 lastDataSize)
{
	if (lastDataSize > minConstantBufferAlignment)
	{
		writeIndex += lastDataSize;
		writeIndex = DirectX::AlignUp(writeIndex, minConstantBufferAlignment);
	}
	else
	{
		writeIndex = DirectX::AlignUp(writeIndex + 1, minConstantBufferAlignment);
		Sleep(2);
	}
}