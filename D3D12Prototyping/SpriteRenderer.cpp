#include "pch.h"
#include "SpriteRenderer.h"
#include "VertexTypes.h"
#include "DeviceResources.h"
#include "LinearConstantBuffer.h"

using namespace DirectX;

SpriteRenderer::SpriteRenderer(ID3D12Device* pGPU, GPUQueue* pCopyEngine, GPUDescriptorHeap* pGlobalHeap, DX::DeviceResources* pEngine)
{
	GPU = pGPU;
	CopyEngine = pCopyEngine;
	CopyCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_COPY);
	GlobalSRV_UAV_CBV = pGlobalHeap;
	renderPassInProgress = false;
	Engine = pEngine;
}

SpriteRenderer::~SpriteRenderer()
{
}

void SpriteRenderer::Initialize(uint32 backBufferCount)
{
	numBackbuffers = backBufferCount;
	GraphicsCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
	GraphicsQueue = new GPUQueue(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);

	//InitializeGPUMemory();
	InitializePipelineState();
}

void SpriteRenderer::BeginRenderPass(uint32 frameIndex, Matrix cameraTransform, ID3D12GraphicsCommandList* cmd)
{
	assert(renderPassInProgress == false);
	this->frameIndex = frameIndex;
	pCurrentCommandList = cmd;

	auto frameMemory = Engine->GetPerFrameMemory(frameIndex);

	cameraTransform = cameraTransform.Transpose();
	auto cameraCBV = frameMemory->Write(&cameraTransform, sizeof(Matrix));

	renderPassInProgress = true;

	cmd->SetGraphicsRootSignature(RootSignature);
	cmd->SetGraphicsRootConstantBufferView(0, cameraCBV);
	cmd->SetPipelineState(SpritePipelineState);

	ID3D12DescriptorHeap* pGlobalHeap = GlobalSRV_UAV_CBV->HeapHandle();
	cmd->SetDescriptorHeaps(1, &pGlobalHeap);
	cmd->RSSetViewports(1, &gameViewport);
	cmd->RSSetScissorRects(1, &gameScissorRect);

	cmd->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmd->IASetVertexBuffers(0, 1, &vbView);
	cmd->IASetIndexBuffer(&ibView);
}

void SpriteRenderer::EndRenderPass()
{
	assert(renderPassInProgress == true);

	// 10-11-2021 -- TODO: determine if anything else needs to go here

	renderPassInProgress = false;
}

void SpriteRenderer::RenderSprite(D3D12_GPU_DESCRIPTOR_HANDLE texture, Vector2 position)
{
	assert(pCurrentCommandList != nullptr);

	float zVal = 1.0f;
	Matrix spriteTransform = Matrix::CreateTranslation(Vector3(position.x, position.y, zVal));
	spriteTransform *= Matrix::CreateOrthographicOffCenter(0.0f, Engine->GetScreenViewport().Width, Engine->GetScreenViewport().Height, 0.0f, 0.0f, 100.0f);
	spriteTransform = spriteTransform.Transpose();

	auto transformCBV = Engine->GetPerFrameMemory(frameIndex)->Write(&spriteTransform, sizeof(Matrix));

	pCurrentCommandList->SetGraphicsRootConstantBufferView(1, transformCBV);
	pCurrentCommandList->SetGraphicsRootDescriptorTable(2, texture);

	pCurrentCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void SpriteRenderer::SetViewportAndScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	gameViewport = viewport;
	gameScissorRect = scissor;
}

//void SpriteRenderer::InitializeGPUMemory()
//{
//	for (int i = 0; i < numBackbuffers; ++i)
//	{
//		LinearConstantBuffer* newBuffer = new LinearConstantBuffer(GPU, 4);
//		PerFrameMem.push_back(newBuffer);
//	}
//}

void SpriteRenderer::InitializePipelineState()
{
	{ //create pipeline root signature
		CD3DX12_ROOT_PARAMETER1 rootParameters[3];

		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		rootParameters[2].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		//2 constant buffer views for vertex shader
		//1 descriptor table (1x srv descriptor) for pixel shader

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob* signature = nullptr;
		ID3DBlob* error = nullptr;
		D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);

		GPU->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature));
	}

	{//load shaders and create pipeline state
		CD3DX12_SHADER_BYTECODE vsBytecode{};
		CD3DX12_SHADER_BYTECODE psBytecode{};

		{
#include "compiled_shaders\sprite_vs.h"
			vsBytecode = CD3DX12_SHADER_BYTECODE(g_main, _countof(g_main));
		}

		{
#include "compiled_shaders\sprite_ps.h"
			psBytecode = CD3DX12_SHADER_BYTECODE(g_main, _countof(g_main));
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC sd{};
		CD3DX12_RASTERIZER_DESC spriteRasterizerState(D3D12_FILL_MODE_SOLID,
			D3D12_CULL_MODE_NONE,
			FALSE,
			D3D12_DEFAULT_DEPTH_BIAS,
			D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
			D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			FALSE,
			FALSE,
			FALSE,
			0,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

		sd.InputLayout = DirectX::VertexPositionTexture::InputLayout;
		sd.pRootSignature = RootSignature;
		sd.VS = vsBytecode;
		sd.PS = psBytecode;
		sd.RasterizerState = spriteRasterizerState;
		sd.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		sd.DepthStencilState.DepthEnable = FALSE;
		sd.DepthStencilState.StencilEnable = FALSE; //no depth or stencil required for sprite rendering
		sd.SampleMask = UINT_MAX;
		sd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		sd.NumRenderTargets = 1;
		//sd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.RTVFormats[0] = Engine->GetBackBufferFormat();
		sd.SampleDesc.Count = 1;

		if (FAILED(GPU->CreateGraphicsPipelineState(&sd, IID_PPV_ARGS(&SpritePipelineState))))
			throw std::runtime_error("failed to create sprite pipeline state!");
	}


	{
		//initialize vertex and index buffers
		// [GOOD CANDIDATE FOR RUNNING IN BACKGROUND THREAD WHILE PIPELINE STATE IS BEING CREATED]
		SpriteVB = new GPUBuffer(GPU);
		SpriteIB = new GPUBuffer(GPU);
		UploadBuffer = new GPUBuffer(GPU);

		size_t vbSize = sizeof(DirectX::VertexPositionTexture) * 4;
		SpriteVB->Create(vbSize,
			D3D12_RESOURCE_STATE_COPY_DEST);

		size_t ibSize = sizeof(uint32) * 6;
		SpriteIB->Create(ibSize,
			D3D12_RESOURCE_STATE_COPY_DEST);

		const uint32 uploadBufferSize = (1024 * 1024) * 4;
		UploadBuffer->Create(uploadBufferSize,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			BUFFER_FLAG_LIFETIME_MAP,
			true);

		float zVal = 1.0f;
		DirectX::VertexPositionTexture quadVerts[4] =
		{
			{XMFLOAT3(-1.0f,1.0f,zVal),XMFLOAT2(0.0f,1.0f)},
			{XMFLOAT3(1.0f,1.0f,zVal),XMFLOAT2(0.0f,0.0f)},
			{XMFLOAT3(-1.0f,-1.0f,zVal), XMFLOAT2(1.0f,1.0f)},
			{XMFLOAT3(1.0f,-1.0f,zVal), XMFLOAT2(1.0f,0.0f)}
		};

		uint32 quadIndices[6] = { 0,1,2, 2,3,0 };

		auto GPUMem = UploadBuffer->Map();
		auto cmd = CopyCommandAllocator->GetCommandList();

		memcpy(GPUMem, &quadVerts, vbSize);
		cmd->CopyBufferRegion(SpriteVB->Handle(), 0, UploadBuffer->Handle(), 0, vbSize);
		CopyEngine->Execute(cmd);
		CopyEngine->Flush();

		cmd = CopyCommandAllocator->GetCommandList();
		memcpy(GPUMem, &quadIndices, ibSize);
		cmd->CopyBufferRegion(SpriteIB->Handle(), 0, UploadBuffer->Handle(), 0, ibSize);
		CopyEngine->Execute(cmd);
		CopyEngine->Flush(); //currently uploading data in two passes for debugging

		cmd = GraphicsCommandAllocator->GetCommandList();

		SpriteVB->StateTransition(cmd, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		SpriteIB->StateTransition(cmd, D3D12_RESOURCE_STATE_INDEX_BUFFER); //transition buffers to required state
		GraphicsQueue->Execute(cmd);
		GraphicsQueue->Flush();

		vbView = {};
		vbView.BufferLocation = SpriteVB->GetGPUAddress();
		vbView.SizeInBytes = vbSize;
		vbView.StrideInBytes = sizeof(DirectX::VertexPositionTexture);

		ibView = {};
		ibView.BufferLocation = SpriteIB->GetGPUAddress();
		ibView.Format = DXGI_FORMAT_R32_UINT;
		ibView.SizeInBytes = ibSize;
	}
}