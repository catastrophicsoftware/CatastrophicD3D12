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

	uint32 NumVertices() const;
	uint32 NumIndices() const;
	D3D12_PRIMITIVE_TOPOLOGY Topology() const;

	//HRESULT Create(int NumVertices, int VertexStride, int NumIndices, D3D11_PRIMITIVE_TOPOLOGY Topology, void* pVertices, void* pIndices);

	HRESULT Create(std::string MeshFile);

private:
	uint32 _NumVertices;
	uint32 _NumIndices;
	uint32 VertexStride;
	uint32 VertexOffset = 0;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	DXGI_FORMAT IndexFormat;

	std::vector<VertexPositionNormalTexture> Vertices;
	std::vector<uint32> Indices;

	void ProcessAssimpMeshNode(aiNode* pNode, const aiScene* pScene);
};