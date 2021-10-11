#include "pch.h"
#include "LinearConstantBuffer.h"

LinearConstantBuffer::LinearConstantBuffer()
{
	GPU = nullptr;
	writeIndex = 0;
	BaseAddress = {};
	buffer = nullptr;
	pBaseGPUMem = nullptr;
	pWritePtr = nullptr;
}

LinearConstantBuffer::LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB, ID3D12Heap* targetHeap, uint64 heapOffset)
{
	writeIndex = 0;
	this->GPU = GPU;
	BaseAddress = {};
	Initialize(sizeInMB, targetHeap, heapOffset);
}

LinearConstantBuffer::LinearConstantBuffer(ID3D12Device* GPU, uint64 sizeInMB)
{
	writeIndex = 0;
	this->GPU = GPU;
	BaseAddress = {};
	Initialize(sizeInMB);
}

LinearConstantBuffer::~LinearConstantBuffer()
{
	Destroy();
}

void LinearConstantBuffer::RegisterFence(InflightGPUWork workHandle)
{

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
	if (!fence.IsComplete())
	{
		//FRAME RATE ANNIHILATING CPU/GPU SYNC POINT
		//AVOID HITTING AT ALL COSTS
		fence.BlockUntilComplete(); //wait until previous access is complete
	}
	writeIndex = 0;
}

void LinearConstantBuffer::Destroy()
{
	//Warning! no protection against in-flight gpu usage
	//this buffer may be being used at this point
	buffer->Release();
	writeIndex = 0;
}


uint64 LinearConstantBuffer::GetConsumedMemory() const
{
	return writeIndex;
}

void LinearConstantBuffer::Initialize(uint64 sizeMB, ID3D12Heap* targetHeap, uint64 heapOffset)
{
	uint64 bufferSize = (1024 * 1024) * sizeMB;
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
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

void LinearConstantBuffer::Initialize(uint64 sizeMB)
{
	uint64 bufferSize = (1024 * 1024) * sizeMB;
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);

	if (FAILED(GPU->CreateCommittedResource(&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer))))
	{
		throw new std::runtime_error("failed to create dynamic constant buffer!");
	}
	else
	{
		BaseAddress = buffer->GetGPUVirtualAddress();
		CD3DX12_RANGE readRange(0, 0); //write only
		buffer->Map(0, &readRange, (void**)&pBaseGPUMem);
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