#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

uint32 Mesh::VertexCount() const
{
	return NumVertices;
}

uint32 Mesh::IndexCount() const
{
	return NumIndices;
}

D3D12_PRIMITIVE_TOPOLOGY Mesh::Topology() const
{
	return topology;
}

void Mesh::CreateBufferViews(D3D12_GPU_VIRTUAL_ADDRESS vbAddr, D3D12_GPU_VIRTUAL_ADDRESS ibAddr)
{	
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = vbAddr;
		view.SizeInBytes = NumVertices * sizeof(VertexPositionNormalTexture);
		view.StrideInBytes = sizeof(VertexPositionNormalTexture);

		vertexBufferView = view;
	}

	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = ibAddr;
		view.Format = IndexFormat;
		view.SizeInBytes = sizeof(uint32) * NumIndices;

		indexBufferView = view;
	}
}