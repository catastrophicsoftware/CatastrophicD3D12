#pragma once
#include "Mesh.h"
#include "LinearConstantBuffer.h"
#include "GPUBuffer.h"

struct CBMaterial
{
	XMFLOAT4 diffuseColor;
};

struct CBViewProjection
{
	XMMATRIX View;
	XMMATRIX Projection;
};

class Game;

class ForwardRenderer
{
public:
	ForwardRenderer(Game* pEngine);
	~ForwardRenderer();

	void InitializeRenderer();
	void CreatePipelineState();

	void BeginFrame(ID3D12GraphicsCommandList* pCMD, uint32 frameIndex);
	void Render(Mesh* pMesh);
	void EndFrame();
	void Render(const D3D12_VERTEX_BUFFER_VIEW vertexBufferView, const D3D12_INDEX_BUFFER_VIEW indexBufferView, int numVertices);

	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

	void UpdateViewProjection(XMMATRIX view, XMMATRIX projection);
	void UpdateWorldTransform(XMMATRIX world);
private:
	Game* pEngine;

	HRESULT CreateGPUBuffers();

	LinearConstantBuffer* PerFrameConstants;
	ID3D12Device* pDevice;

	ID3D12PipelineState* PipelineState;
	ID3D12RootSignature* RootSignature;

	DXGI_FORMAT BackBufferFormat;

	CBViewProjection viewProjection;

	GPUBuffer* pCBViewProjection;
	GPUBuffer* pCBMaterial;
	uint64* pCBMaterialGPUMemory;

	ID3D12GraphicsCommandList* pCurrentFrameCommandList;
	bool renderPassInProgress;
	uint32 currentFrameIndex;

	UINT64 frameCount;
};