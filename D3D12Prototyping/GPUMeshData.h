#pragma once
#include <d3d11.h>

class GPUMeshData
{
public:
	GPUMeshData(D3D12_GPU_VIRTUAL_ADDRESS vertexGPUAddress, D3D12_GPU_VIRTUAL_ADDRESS indexGPUAddress, uint32 indexCount, uint32 vertexCount, uint32 vertexStride, D3D12_PRIMITIVE_TOPOLOGY topology);
	~GPUMeshData();

	D3D12_VERTEX_BUFFER_VIEW GetVBV() const;
	D3D12_INDEX_BUFFER_VIEW GetIVB() const;
	D3D12_PRIMITIVE_TOPOLOGY GetTopology() const;

	uint32 VertexCount() const;
	uint32 IndexCount() const;
	uint32 VertexStride() const;
private:
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW  indexBufferView;
	D3D12_PRIMITIVE_TOPOLOGY topology;

	uint32 vertexCount;
	uint32 indexCount;
	uint32 vertexStride;
};