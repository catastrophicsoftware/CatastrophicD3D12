#include "pch.h"
#include "ForwardRenderer.h"
#include "Game.h"
#include "GPUMeshData.h"

ForwardRenderer::ForwardRenderer(Game* pEngine)
{
    this->pEngine = pEngine;
    BackBufferFormat = pEngine->GetGPUResources()->GetBackBufferFormat();

    pDevice = pEngine->GetGPUResources()->GetD3DDevice();
    renderPassInProgress = false;
}

ForwardRenderer::~ForwardRenderer()
{
}

void ForwardRenderer::InitializeRenderer()
{
    SCOPED_PERFORMANCE_TIMER("ForwardRenderer::InitializeRenderer()");
    
    CreatePipelineState();
    CreateGPUBuffers();

}

void ForwardRenderer::CreatePipelineState()
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[4];

    XMFLOAT4 magenta = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX); //view,projection
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX); //world
    rootParameters[2].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  //material (CBMaterial struct)

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    rootParameters[3].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


    globalDiffuseSampler = {};
    globalDiffuseSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    globalDiffuseSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    globalDiffuseSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    globalDiffuseSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    globalDiffuseSampler.MipLODBias = 0;
    globalDiffuseSampler.MaxAnisotropy = 0;
    globalDiffuseSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    globalDiffuseSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    globalDiffuseSampler.MinLOD = 0.0f;
    globalDiffuseSampler.MaxLOD = D3D12_FLOAT32_MAX;
    globalDiffuseSampler.ShaderRegister = 0;
    globalDiffuseSampler.RegisterSpace = 0;
    globalDiffuseSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1,&globalDiffuseSampler, rootSignatureFlags);

    ID3DBlob* signature;
    ID3DBlob* error;
    D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature));


    {
        CD3DX12_SHADER_BYTECODE vsBytecode;
        CD3DX12_SHADER_BYTECODE psBytecode;

        //TODO: look into loading shaders from file to reduce function stack space

        {
#include "compiled_shaders\simple_vs.h"
            vsBytecode = CD3DX12_SHADER_BYTECODE(g_main, ARRAYSIZE(g_main));
        }

        {
#include "compiled_shaders\simple_ps.h"
            psBytecode = CD3DX12_SHADER_BYTECODE(g_main, ARRAYSIZE(g_main));
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

        psoDesc.InputLayout = VertexPositionNormalTexture::InputLayout;
        psoDesc.pRootSignature = RootSignature;

        psoDesc.VS = vsBytecode;
        psoDesc.PS = psBytecode;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DSVFormat = pEngine->GetGPUResources()->GetDepthBufferFormat();

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = BackBufferFormat;
        psoDesc.SampleDesc.Count = 1;

        if (FAILED(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PipelineState))))
        {
            throw std::runtime_error("failed to create pipeline state object!");
        }
    }
}

void ForwardRenderer::BeginFrame(ID3D12GraphicsCommandList* pCMD, uint32 frameIndex)
{
    SCOPED_PERFORMANCE_TIMER("ForwardRenderer::BeginFrame()");

    if ((frameCount % 100) == 0)
        PerFrameConstants->Reset(); //kind of hacky, reset per frame buffer every 100 frames, deal with this later


    if (!renderPassInProgress)
    {
        currentFrameIndex = frameIndex;
        PerFrameConstants->Reset(); //TODO: look into this

        pCMD->SetPipelineState(PipelineState);
        pCMD->SetGraphicsRootSignature(RootSignature);
        auto globalSRV = pEngine->GetGlobalSRVHeap()->HeapHandle();
        pCMD->SetDescriptorHeaps(1, &globalSRV);

        pCMD->SetGraphicsRootConstantBufferView(0, pCBViewProjection->GetGPUAddress());
        pCMD->SetGraphicsRootConstantBufferView(2, pCBMaterial->GetGPUAddress());

        renderPassInProgress = true;
        pCurrentFrameCommandList = pCMD;
    }
    else
        throw std::runtime_error("RENDER PASS ALREADY IN PROGRESS!");
}

void ForwardRenderer::Render(Mesh* pMesh)
{
    if (renderPassInProgress)
    {
        auto gpuData = pMesh->GetGPUMeshData();

        auto vbv = gpuData->GetVBV();
        pCurrentFrameCommandList->IASetVertexBuffers(0, 1, &vbv);

        if (gpuData->IndexCount() > 0)
        {
            auto ibv = gpuData->GetIBV();
            pCurrentFrameCommandList->IASetIndexBuffer(&ibv);
        }

        pCurrentFrameCommandList->IASetPrimitiveTopology(gpuData->GetTopology());

        pCurrentFrameCommandList->DrawIndexedInstanced(gpuData->IndexCount(), 1, 0, 0, 0);
    }
    else
        throw std::runtime_error("RENDER PASS ALREADY IN PROGRESS!");
}

void ForwardRenderer::Render(const D3D12_VERTEX_BUFFER_VIEW vertexBufferView,const D3D12_INDEX_BUFFER_VIEW indexBufferView, int numVertices)
{
    assert(vertexBufferView.BufferLocation != 0);

    pCurrentFrameCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    if (indexBufferView.BufferLocation != 0)
    {
        pCurrentFrameCommandList->IASetIndexBuffer(&indexBufferView);

        pCurrentFrameCommandList->DrawIndexedInstanced(3, (numVertices / 3), 0, 0, 0);
    }
    else
    {
        pCurrentFrameCommandList->DrawInstanced(3, (numVertices / 3), 0, 0);
    }
}

void ForwardRenderer::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
{
    if (renderPassInProgress)
    {
        pCurrentFrameCommandList->IASetPrimitiveTopology(topology);
    }
    else
        throw std::runtime_error("RENDER PASS ALREADY IN PROGRESS!");
}

void ForwardRenderer::EndFrame()
{
    if (renderPassInProgress)
    {
        SCOPED_PERFORMANCE_TIMER("ForwardRenderer::EndFrame()");

        //TODO: determine what else to do heres

        renderPassInProgress = false;
        frameCount++;
    }
    else
        throw std::runtime_error("RENDER PASS ALREADY IN PROGRESS!");
}

void ForwardRenderer::UpdateViewProjection(XMMATRIX view, XMMATRIX projection)
{
    viewProjection.View = view;
    viewProjection.Projection = projection;

    uint64* pGPUMemory = pCBViewProjection->Map();
    memcpy(pGPUMemory, &viewProjection, sizeof(CBViewProjection)); //perform copy into GPU memory
}

void ForwardRenderer::UpdateWorldTransform(XMMATRIX world)
{
    if (renderPassInProgress)
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuMemoryOffset = PerFrameConstants->Write(&world, sizeof(XMMATRIX));
        pCurrentFrameCommandList->SetGraphicsRootConstantBufferView(1, gpuMemoryOffset);
    }
    else
        throw std::runtime_error("RENDER PASS ALREADY IN PROGRESS!");
}

HRESULT ForwardRenderer::CreateGPUBuffers()
{
    pCBViewProjection = new GPUBuffer(pDevice);
    pCBViewProjection->Create(sizeof(CBViewProjection), D3D12_RESOURCE_STATE_GENERIC_READ, BUFFER_FLAG_LIFETIME_MAP, true);

    pCBMaterial = new GPUBuffer(pDevice);
    pCBMaterial->Create(sizeof(CBMaterial), D3D12_RESOURCE_STATE_GENERIC_READ, BUFFER_FLAG_LIFETIME_MAP, true);
    pCBMaterialGPUMemory = pCBMaterial->Map(); //cache pointer to material constant buffer GPU memory.

    PerFrameConstants = new LinearConstantBuffer(pDevice, 8);

    return S_OK;
}