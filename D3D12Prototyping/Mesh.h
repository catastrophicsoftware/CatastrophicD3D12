#pragma once
#include "pch.h"
#include "VertexTypes.h"
#include <string.h>
#include <vector>

using namespace DirectX;

class GPUMeshData;
class StaticGeometryBuffer;

class Mesh
{
public:
	Mesh();
	~Mesh();

	void Load(std::string meshFile, StaticGeometryBuffer* staticGeometry);

	void ReleaseCPUGeometryData();

	std::shared_ptr<GPUMeshData> GetGPUMeshData() const;
private:
	std::shared_ptr<GPUMeshData> gpuMeshData;

	std::vector<DirectX::VertexPositionNormalTexture> Vertices;
	std::vector<uint32> Indices;
};