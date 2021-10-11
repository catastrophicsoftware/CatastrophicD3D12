#include "pch.h"
#include "GPUBuffer.h"

GPUBuffer::GPUBuffer()
{
	GPU = nullptr;
	buffer = nullptr;
	created = false;
	mapped = false;
	GPUAddress = {};
	cpuAccessible = false;
	flags = BUFFER_FLAG_NONE;
	state = D3D12_RESOURCE_STATE_COMMON;
	pGPUMemory = nullptr;
	size = 0;
}

GPUBuffer::GPUBuffer(ID3D12Device* pGPU)
{
	GPU = pGPU;
	buffer = nullptr;
	created = false;
	mapped = false;
	GPUAddress = {};
	cpuAccessible = false;
	flags = BUFFER_FLAG_NONE;
	state = D3D12_RESOURCE_STATE_COMMON;
	pGPUMemory = nullptr;
	size = 0;
}

void GPUBuffer::Create(uint64 size, D3D12_RESOURCE_STATES state, CENGINE_BUFFER_FLAGS flags, bool cpuAccess)
{
	createGPUBuffer(size, state, cpuAccess, flags);
}

void GPUBuffer::Create(uint64 sizeInBytes, D3D12_RESOURCE_STATES initialState, CENGINE_BUFFER_FLAGS bufferFlags, bool cpuAccessible,ID3D12Heap* pTargetHeap, uint64 heapOffset)
{
	createGPUBuffer(sizeInBytes, initialState, cpuAccessible, flags, pTargetHeap, heapOffset);
}

void GPUBuffer::StateTransition(ID3D12GraphicsCommandList* pCMD, D3D12_RESOURCE_STATES newState)
{
	if (state == newState)
		return;

	CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer, state, newState);
	pCMD->ResourceBarrier(1, &transitionBarrier);
	//Warning! transition is recorded, but this is an in-flight operation that has not actually completed (or even started) at this point
	//TODO: figure out how to handle these in-flight operations
}

void GPUBuffer::GPUCopyFromBuffer(ID3D12GraphicsCommandList* pCMD, uint64 offset, ID3D12Resource* pSourceBuffer, uint64 sourceOffset, uint64 numBytes, bool preserveState)
{
	D3D12_RESOURCE_STATES prevState = state;

	if (state != D3D12_RESOURCE_STATE_COPY_DEST)
		StateTransition(pCMD, D3D12_RESOURCE_STATE_COPY_DEST); //ensure buffer is ready as copy destination

	pCMD->CopyBufferRegion(buffer, offset, pSourceBuffer, sourceOffset, numBytes);

	if (preserveState) //return this buffer to it's previous resource state before this copy operation
		StateTransition(pCMD, prevState);
}

void GPUBuffer::GPUCopyFromTexture(ID3D12GraphicsCommandList* pCMD)
{
	D3D12_RESOURCE_STATES prevState = state;

	if (state != D3D12_RESOURCE_STATE_COPY_DEST)
		StateTransition(pCMD, D3D12_RESOURCE_STATE_COPY_DEST); //ensure buffer is ready as copy destination


	throw std::runtime_error("NOT IMPLEMENTED!");
}

uint64* GPUBuffer::Map()
{
	assert(cpuAccessible == true);

	if (!mapped)
	{
		void* pTempMem = nullptr;
		CD3DX12_RANGE readRange(0, 0); //write only
		buffer->Map(0, &readRange, &pTempMem);
		pGPUMemory = static_cast<uint64*>(pTempMem);
		mapped = true;
		return pGPUMemory;
	}
	return pGPUMemory; //resource is already mapped, simply return pointer to gpu memory
}

void GPUBuffer::Unmap()
{
	assert(cpuAccessible == true);
	if (mapped)
	{
		buffer->Unmap(0, nullptr);
		mapped = false;
		pGPUMemory = nullptr;
	}
}

bool GPUBuffer::Created() const
{
	return created;
}

D3D12_GPU_VIRTUAL_ADDRESS GPUBuffer::GetGPUAddress()
{
	return GPUAddress;
}

ID3D12Resource* GPUBuffer::Handle() const
{
	return buffer;
}

void GPUBuffer::WriteElementAtIndex(uint64 elementSize, uint64 index, void* pData)
{
	assert(cpuAccessible);
	assert(((elementSize * index) + elementSize) <= size); //ensure there is space to write an element after the current index

	if (!mapped)
		Map();

	uint64* pDest = pGPUMemory + (elementSize * index);
	memcpy(pDest, pData, elementSize);
}

D3D12_GPU_VIRTUAL_ADDRESS GPUBuffer::GetElementAddress(uint64 elementSize, uint64 index)
{
	return GPUAddress + (elementSize * index);
}

void GPUBuffer::createGPUBuffer(uint64 sizeInBytes,D3D12_RESOURCE_STATES initialState,bool cpuAccessible,CENGINE_BUFFER_FLAGS flags, ID3D12Heap* pTargetHeap, uint64 heapOffset)
{
	size = sizeInBytes;
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);

	if (pTargetHeap)
	{
		this->cpuAccessible = cpuAccessible; //for placed resource, we will have to trust this from the sender
		//placed resource
		if (FAILED(GPU->CreatePlacedResource(pTargetHeap,
			heapOffset,
			&bufferDesc,
			initialState,
			nullptr,
			IID_PPV_ARGS(&buffer))))
		{
			throw std::runtime_error("failed to create gpu buffer!");
		}
		else
		{
			created = true;
			state = initialState; //resource is now properly created and in this state
			GPUAddress = buffer->GetGPUVirtualAddress();

			if (flags & BUFFER_FLAG_LIFETIME_MAP && cpuAccessible)
			{
				//this buffer will be mapped for it's lifetime

				CD3DX12_RANGE readRange(0, 0); //write only
				void* pTempGPUMem = nullptr;
				if (!FAILED(buffer->Map(0, &readRange, &pTempGPUMem)))
				{
					pGPUMemory = static_cast<uint64*>(pTempGPUMem);
					mapped = true;
				}
			}
		}
	}
	else
	{
		//comitted resource
		D3D12_HEAP_TYPE heapType = cpuAccessible ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
		//Warning: read-back not currently supported!

		CD3DX12_HEAP_PROPERTIES heapProperties(heapType);
		if (FAILED(GPU->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			initialState,
			nullptr,
			IID_PPV_ARGS(&buffer))))
		{
			throw std::runtime_error("failed to create gpu buffer!");
		}
		else
		{
			created = true;
			state = initialState; //resource is now properly created and in this state
			GPUAddress = buffer->GetGPUVirtualAddress();

			if ((flags & BUFFER_FLAG_LIFETIME_MAP) && cpuAccessible)
			{
				//this buffer will be mapped for it's lifetime
				Map();
			}
		}
	}
}