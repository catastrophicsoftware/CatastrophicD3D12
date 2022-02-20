#include "pch.h"
#include "Mesh.h"
#include <fstream>
#include "obj_loader.h"
#include "GPUMeshData.h"
#include "StaticGeometryBuffer.h"

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

void Mesh::Load(std::string meshFile, StaticGeometryBuffer* staticGeometry)
{
	SCOPED_PERFORMANCE_TIMER("Mesh::Load()");

	std::unique_ptr<objl::Loader> meshLoader = std::make_unique<objl::Loader>();

	meshLoader->LoadFile(meshFile);

	if (meshLoader->LoadedMeshes.size() == 0)
	{
		throw new std::runtime_error("No meshes loaded!");
	}
	else
	{
		for (int i = 0; i < meshLoader->LoadedVertices.size(); ++i)
		{
			objl::Vertex currentVertex = meshLoader->LoadedVertices[i];

			
			Vertices.push_back(VertexPositionNormalTexture(XMFLOAT3(currentVertex.Position.X, currentVertex.Position.Y, currentVertex.Position.Z),
				XMFLOAT3(currentVertex.Normal.X, currentVertex.Normal.Y, currentVertex.Normal.Z),
				XMFLOAT2(currentVertex.TextureCoordinate.X, currentVertex.TextureCoordinate.Y)));

		}

		for (int i = 0; i < meshLoader->LoadedIndices.size(); ++i)
		{
			Indices.push_back(meshLoader->LoadedIndices[i]);
		}

		auto vertexGPUMemoryHandle = staticGeometry->WriteVertices(Vertices.data(), sizeof(VertexPositionNormalTexture), Vertices.size());
		auto indexGPUMemoryHandle = staticGeometry->WriteIndices(Indices.data(), Indices.size());

		gpuMeshData = std::make_shared<GPUMeshData>(meshFile, vertexGPUMemoryHandle, indexGPUMemoryHandle, Indices.size(), Vertices.size(), sizeof(VertexPositionNormalTexture), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ReleaseCPUGeometryData(); //cpu copy of geometry no longer needed
	}
}



void Mesh::ReleaseCPUGeometryData()
{
	if (Vertices.size() > 0)
	{
		std::vector<VertexPositionNormalTexture> emptyVerts;
		Vertices.swap(emptyVerts);
	}
	if (Indices.size() > 0)
	{
		std::vector<uint32> emptyIndices;
		Indices.swap(emptyIndices);
	}
}

std::shared_ptr<GPUMeshData> Mesh::GetGPUMeshData() const
{
	return gpuMeshData;
}
