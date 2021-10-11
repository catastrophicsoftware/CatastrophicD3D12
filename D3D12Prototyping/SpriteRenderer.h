#pragma once
#include "pch.h"
#include "GPUCommandQueue.h"
#include "GPUCommandAllocator.h"
#include "LinearConstantBuffer.h"
#include "GPUBuffer.h"
#include "GPUDescriptorHeap.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class SpriteRenderer
{
public:
	SpriteRenderer(ID3D12Device* pGPU, GPUQueue* pCopyEngine, GPUDescriptorHeap* pGlobalHeap);
	~SpriteRenderer();

	void Initialize(uint32 backBufferCount);

	void BeginRenderPass(uint32 frameIndex, ID3D12GraphicsCommandList* cmd, Matrix cameraTransform);
	void EndRenderPass();

	void RenderSprite(D3D12_GPU_DESCRIPTOR_HANDLE texture, Vector2 position);

	void SetViewportAndScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor);
private:
	ID3D12Device* GPU;
	GPUQueue* GPUWorkQueue;
	GPUCommandAllocator* CommandAllocator;

	GPUQueue* CopyEngine;
	GPUCommandAllocator* CopyCommandAllocator;

	uint32 numBackbuffers;

	ID3D12PipelineState* SpritePipelineState;
	ID3D12RootSignature* RootSignature;

	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	GPUBuffer* SpriteVB;
	GPUBuffer* SpriteIB;

	GPUBuffer* SpriteCameraCB;

	GPUBuffer* UploadBuffer;
	GPUDescriptorHeap* GlobalSRV_UAV_CBV;

	D3D12_VIEWPORT gameViewport;
	D3D12_RECT gameScissorRect;
	
	bool renderPassInProgress;

	std::vector<LinearConstantBuffer*> PerFrameMem;
	void InitializeGPUMemory();
	void InitializePipelineState();
};