#include "pch.h"
#include "Mesh.h"
#include <fstream>
#include "obj_loader.h"

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

void Mesh::Load(std::string meshFile)
{
	SCOPED_PERFORMANCE_TIMER("Mesh::Load()");

	objl::Loader* meshLoader = new objl::Loader();
	meshLoader->LoadFile(meshFile);

	if (meshLoader->LoadedMeshes.size() == 0)
	{
		throw new std::runtime_error("No meshes loaded!");
	}
	else
	{
		NumVertices = meshLoader->LoadedVertices.size();
		NumIndices = meshLoader->LoadedIndices.size();
		VertexStride = sizeof(objl::Vertex);

		for (int i = 0; i < meshLoader->LoadedVertices.size(); ++i)
		{
			objl::Vertex currentVertex = meshLoader->LoadedVertices[i];

			Vertices.push_back(VertexPositionNormalTexture(XMFLOAT3(currentVertex.Position.X, currentVertex.Position.Y, currentVertex.Position.Z),
				XMFLOAT3(currentVertex.Normal.X, currentVertex.Normal.Y, currentVertex.Normal.Z),
				XMFLOAT2(currentVertex.Position.X, currentVertex.Position.Y)));

		}

		for (int i = 0; i < meshLoader->LoadedIndices.size(); ++i)
		{
			Indices.push_back(meshLoader->LoadedIndices[i]);
		}

		IndexFormat = DXGI_FORMAT_R32_UINT;
	}
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

DirectX::VertexPositionNormalTexture* Mesh::GetVertexDataPointer()
{
	return Vertices.data();
}

uint32* Mesh::GetIndexDataPointer()
{
	return Indices.data();
}