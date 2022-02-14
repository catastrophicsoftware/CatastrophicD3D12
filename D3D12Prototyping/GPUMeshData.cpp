#include "pch.h"
#include "GPUMeshData.h"


GPUMeshData::GPUMeshData(D3D12_GPU_VIRTUAL_ADDRESS vertexGPUAddress, D3D12_GPU_VIRTUAL_ADDRESS indexGPUAddress, uint32 indexCount, uint32 vertexCount, uint32 vertexStride, D3D12_PRIMITIVE_TOPOLOGY topology)
{
	assert(vertexCount > 0);
	assert(vertexStride > 0);

	vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexGPUAddress;
	vertexBufferView.SizeInBytes = vertexCount * vertexStride;
	vertexBufferView.StrideInBytes = vertexStride;

	indexBufferView = {};
	indexBufferView.BufferLocation = indexGPUAddress;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(uint32) * indexCount;

	this->topology = topology;
	this->vertexCount = vertexCount;
	this->indexCount = indexCount;
	this->vertexStride = vertexStride;
}

GPUMeshData::~GPUMeshData()
{
}

D3D12_VERTEX_BUFFER_VIEW GPUMeshData::GetVBV() const
{
	return vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW GPUMeshData::GetIVB() const
{
	return indexBufferView;
}

D3D12_PRIMITIVE_TOPOLOGY GPUMeshData::GetTopology() const
{
	return topology;
}

uint32 GPUMeshData::VertexCount() const
{
	return vertexCount;
}

uint32 GPUMeshData::IndexCount() const
{
	return indexCount;
}

uint32 GPUMeshData::VertexStride() const
{
	return vertexStride;
}