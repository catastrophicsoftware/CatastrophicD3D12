#include "pch.h"
#include "SpriteRenderer.h"
#include "VertexTypes.h"

using namespace DirectX;

SpriteRenderer::SpriteRenderer(ID3D12Device* pGPU, GPUQueue* pCopyEngine, GPUDescriptorHeap* pGlobalHeap)
{
	GPU = pGPU;
	CopyEngine = pCopyEngine;
	CopyCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_COPY);
	GlobalSRV_UAV_CBV = pGlobalHeap;
	renderPassInProgress = false;
}

SpriteRenderer::~SpriteRenderer()
{
}

void SpriteRenderer::Initialize(uint32 backBufferCount)
{
	numBackbuffers = backBufferCount;

	InitializeGPUMemory();
	InitializePipelineState();
}

void SpriteRenderer::BeginRenderPass(uint32 frameIndex, ID3D12GraphicsCommandList* cmd, Matrix cameraTransform)
{
	assert(renderPassInProgress == false);
	auto frameMemory = PerFrameMem[frameIndex];
	frameMemory->Reset();
	auto cameraCBV = frameMemory->Write(&cameraTransform, sizeof(Matrix));

	

	renderPassInProgress = true;

	cmd->SetGraphicsRootSignature(RootSignature);
	cmd->SetGraphicsRootConstantBufferView(0, cameraCBV);

	ID3D12DescriptorHeap* pGlobalHeap = GlobalSRV_UAV_CBV->HeapHandle();
	cmd->SetDescriptorHeaps(1, &pGlobalHeap);
	cmd->RSSetViewports(1, &gameViewport);
	cmd->RSSetScissorRects(1, &gameScissorRect);
}

void SpriteRenderer::EndRenderPass()
{
	assert(renderPassInProgress == true);

	//TODO: figure out what/if anything needs to be done here
	//possibly return an InflightGPUWork handle

	renderPassInProgress = false;
}

void SpriteRenderer::SetViewportAndScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	gameViewport = viewport;
	gameScissorRect = scissor;
}

void SpriteRenderer::InitializeGPUMemory()
{
	for (int i = 0; i < numBackbuffers; ++i)
	{
		LinearConstantBuffer* newBuffer = new LinearConstantBuffer(GPU, 4);
		PerFrameMem.push_back(newBuffer);
	}
}

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
			D3D12_CULL_MODE_BACK,
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
		sd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
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
		SpriteCameraCB = new GPUBuffer(GPU);

		SpriteCameraCB->Create(sizeof(Matrix) * 3,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			BUFFER_FLAG_LIFETIME_MAP,
			true);

		size_t vbSize = sizeof(DirectX::VertexPositionTexture) * 3;
		SpriteVB->Create(vbSize,
			D3D12_RESOURCE_STATE_COPY_DEST);

		size_t ibSize = sizeof(uint32) * 6;
		SpriteIB->Create(ibSize,
			D3D12_RESOURCE_STATE_COPY_DEST);

		const uint32 uploadBufferSize = (1024 * 1024) * 2;
		UploadBuffer->Create(uploadBufferSize,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			BUFFER_FLAG_LIFETIME_MAP);

		float zVal = 0.0f;
		DirectX::VertexPositionTexture quadVerts[4] =
		{
			{XMFLOAT3(-0.25f,1.0f,zVal),XMFLOAT2(0.0f,1.0f)},
			{XMFLOAT3(0.25f,1.0f,zVal),XMFLOAT2(0.0f,0.0f)},
			{XMFLOAT3(-0.25f,-1.0f,zVal), XMFLOAT2(1.0f,1.0f)},
			{XMFLOAT3(0.25f,-1.0f,zVal), XMFLOAT2(1.0f,0.0f)}
		};

		uint32 quadIndices[6] = { 0,1,2, 2,3,0 };

		auto GPUMem = UploadBuffer->Map();

		memcpy(GPUMem, &quadVerts, vbSize);
		GPUMem += vbSize;
		memcpy(GPUMem, &quadIndices, ibSize);

		auto cmd = CopyCommandAllocator->GetCommandList();
		cmd->CopyBufferRegion(SpriteVB->Handle(), 0, UploadBuffer->Handle(), 0, vbSize);
		cmd->CopyBufferRegion(SpriteIB->Handle(), 0, UploadBuffer->Handle(), vbSize, ibSize);

		SpriteVB->StateTransition(cmd, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		SpriteIB->StateTransition(cmd, D3D12_RESOURCE_STATE_INDEX_BUFFER); //transition buffers to required state

		CopyEngine->Execute(cmd);
		CopyEngine->Flush();

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