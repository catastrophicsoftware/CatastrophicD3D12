#include "pch.h"
#include "StaticGeometryBuffer.h"

StaticGeometryBuffer::StaticGeometryBuffer(ID3D12Device* pDevice, GPUQueue* pCopyQueue)
{
	GPU = pDevice;
	vertexIndex = 0;
	indexIndex = 0;
	locked = false;

	CopyQueue = pCopyQueue;
	CopyCmdAlloc = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_COPY);
	DctCmdAlloc = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
	GraphicsQueue = new GPUQueue(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

StaticGeometryBuffer::~StaticGeometryBuffer()
{
}

HRESULT StaticGeometryBuffer::Create(uint64 sizeMB)
{
	CD3DX12_HEAP_PROPERTIES staticHeap(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES dynHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((1024*1024) * sizeMB);

	{
		//create vertex buffer and vertex staging buffer
		GPU->CreateCommittedResource(&staticHeap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&StaticVertexBuffer));
		VertexGPUBaseAddr = StaticVertexBuffer->GetGPUVirtualAddress();

		GPU->CreateCommittedResource(&dynHeap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&StagingVertexBuffer));
		CD3DX12_RANGE readRange(0, 0);
		StagingVertexBuffer->Map(0, &readRange, &pVertexStagingMemory);
	}

	{
		GPU->CreateCommittedResource(&staticHeap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&StaticIndexBuffer));
		IndexGPUBaseAddr = StaticIndexBuffer->GetGPUVirtualAddress();

		GPU->CreateCommittedResource(&dynHeap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&StagingIndexBuffer));
		CD3DX12_RANGE readRange(0, 0);
		StagingIndexBuffer->Map(0, &readRange, &pIndexStagingMemory);
	}

	return S_OK;
}

D3D12_GPU_VIRTUAL_ADDRESS StaticGeometryBuffer::WriteVertices(void* pVertexData, uint64 vertexStride, uint64 numVertices)
{
	if (!locked)
	{
		D3D12_GPU_VIRTUAL_ADDRESS outGPUAddr = StaticVertexBuffer->GetGPUVirtualAddress() + vertexIndex;
		UINT64 currentOffset = vertexIndex;

		vertexIndex += (vertexStride * numVertices); //advance vertex index

		//load data into vertex staging buffer
		uint8_t* VertexGPUDest = (uint8_t*)pVertexStagingMemory + currentOffset;
		memcpy(VertexGPUDest, pVertexData, vertexStride * numVertices);

		return outGPUAddr; //this address will be valid once contents are committed to static memory
	}
	return 0; //resource is locked, cannot accept new vertices
}

D3D12_GPU_VIRTUAL_ADDRESS StaticGeometryBuffer::WriteIndices(void* pIndexData, uint64 indexCount)
{
	if (!locked)
	{
		D3D12_GPU_VIRTUAL_ADDRESS outGPUAddr = StaticIndexBuffer->GetGPUVirtualAddress() + indexIndex;
		UINT64 currentOffset = indexIndex; //record offset of return address

		indexIndex += indexCount; //advance index

		uint8_t* IndexGPUDest = (uint8_t*)pIndexStagingMemory + currentOffset;
		memcpy(IndexGPUDest, pIndexData, sizeof(uint32) * indexCount);

		return outGPUAddr; //this address will be valid once contents are committed to static memory
	}
	return 0; //resource is locked, cannot accept new indices
}

ID3D12Resource* StaticGeometryBuffer::GetVertexBuffer() const
{
	return StaticVertexBuffer;
}

ID3D12Resource* StaticGeometryBuffer::GetIndexBuffer() const
{
	return StaticIndexBuffer;
}

void StaticGeometryBuffer::Commit()
{
	if (!locked)
	{
		auto copyCMD = CopyCmdAlloc->GetCommandList();

		copyCMD->CopyBufferRegion(StaticVertexBuffer, 0, StagingVertexBuffer, 0, vertexIndex); //copy vertex data
		copyCMD->CopyBufferRegion(StaticIndexBuffer, 0, StagingIndexBuffer, 0, sizeof(uint32) * indexIndex);

		auto workVal = CopyQueue->ExecuteCommandList(copyCMD);
		CopyQueue->WaitForFenceCPUBlocking(workVal);

		auto transitionCMD = DctCmdAlloc->GetCommandList();
		CD3DX12_RESOURCE_BARRIER vbBarrier = CD3DX12_RESOURCE_BARRIER::Transition(StaticVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 0);
		CD3DX12_RESOURCE_BARRIER ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(StaticIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER, 0);
		CD3DX12_RESOURCE_BARRIER barriers[2] = { vbBarrier,ibBarrier };

		transitionCMD->ResourceBarrier(2, barriers);
		workVal = GraphicsQueue->ExecuteCommandList(transitionCMD);
		GraphicsQueue->WaitForFenceCPUBlocking(workVal);
		locked = true;
	}
}

void StaticGeometryBuffer::Reset()
{
	if (locked)
	{
		vertexIndex = 0;
		indexIndex = 0;

		CD3DX12_RESOURCE_BARRIER vbBarrier = CD3DX12_RESOURCE_BARRIER::Transition(StaticVertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST, 0);
		CD3DX12_RESOURCE_BARRIER ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(StaticIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST, 0);
		CD3DX12_RESOURCE_BARRIER barriers[2] = { vbBarrier,ibBarrier };

		auto transitionCMD = DctCmdAlloc->GetCommandList();
		transitionCMD->ResourceBarrier(2, barriers); //transition buffers to copy destination optimized

		auto workVal = GraphicsQueue->ExecuteCommandList(transitionCMD);
		GraphicsQueue->WaitForFenceCPUBlocking(workVal); //wait for transition to complete
		locked = false;
		//resource is now ready for data to be written into staging buffers and re-comitted to dedicated gpu memory
	}
}