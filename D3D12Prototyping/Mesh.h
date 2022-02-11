#pragma once
#include "pch.h"
#include "VertexTypes.h"
#include <string.h>
#include <vector>

using namespace DirectX;

class Mesh
{
public:
	Mesh();
	~Mesh();

	void Load(std::string meshFile);

	uint32 VertexCount() const;
	uint32 IndexCount() const;
	D3D12_PRIMITIVE_TOPOLOGY Topology() const;
	void CreateBufferViews(D3D12_GPU_VIRTUAL_ADDRESS vbAddr, D3D12_GPU_VIRTUAL_ADDRESS ibAddr);

	DirectX::VertexPositionNormalTexture* GetVertexDataPointer();
	uint32* GetIndexDataPointer();
private:
	uint32 NumVertices;
	uint32 NumIndices;
	uint32 VertexStride;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	DXGI_FORMAT IndexFormat;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	std::vector<DirectX::VertexPositionNormalTexture> Vertices;
	std::vector<uint32> Indices;
};