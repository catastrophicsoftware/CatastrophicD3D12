#pragma once
#include "pch.h"
#include "VertexTypes.h"
#include <assimp\scene.h>
#include <assimp\mesh.h>
#include <assimp\cimport.h>
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <string.h>
#include <vector>

using namespace DirectX;

class Mesh
{
public:
	Mesh();
	~Mesh();

	uint32 VertexCount() const;
	uint32 IndexCount() const;
	D3D12_PRIMITIVE_TOPOLOGY Topology() const;

	HRESULT Load(std::string MeshFile);

	void SetGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);
private:
	uint32 NumVertices;
	uint32 NumIndices;
	uint32 VertexStride;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	DXGI_FORMAT IndexFormat;

	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;

	std::vector<VertexPositionNormalTexture> Vertices;
	std::vector<uint32> Indices;

	void ProcessAssimpMeshNode(aiNode* pNode, const aiScene* pScene);
};