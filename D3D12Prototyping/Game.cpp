//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "VertexTypes.h"
#include <d3dcompiler.h>

#pragma comment(lib,"D3DCompiler.lib")

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;


Game::Game() noexcept(false) : BackgroundPool()
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    GPU = m_deviceResources->GetD3DDevice();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    InitializeInput();

    InitializeDescriptorHeap();
    InitializePipeline();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        GamepadState = Controller->GetState(0);
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update"); // See pch.h for info

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    auto bufferCount = m_deviceResources->GetBackBufferCount();
    UINT frameIndex = (m_deviceResources->GetCurrentFrameIndex() % bufferCount);

    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render"); // See pch.h for info
    //-----------------------------------------------------------------------------------------
    commandList->SetPipelineState(PSO);
    commandList->SetGraphicsRootSignature(RootSig);

    ID3D12DescriptorHeap* pDescriptorHeap = Descriptors->Heap();
    commandList->SetDescriptorHeaps(1, &pDescriptorHeap);
    commandList->SetGraphicsRootDescriptorTable(0, Descriptors->GetFirstGpuHandle());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vbView);
    commandList->DrawInstanced(3, 1, 0, 0);

    //------------------------------------------------------------------------------------------
    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present"); // See pch.h for info
    m_deviceResources->Present();
    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear"); // See pch.h for info

    // Clear the views.
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::DarkSlateBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::InitializeInput()
{
    Controller = std::make_unique<GamePad>();
    Keyboard = std::make_unique<DirectX::Keyboard>();
}

void Game::InitializePipeline()
{
    CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];

    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    rootParameters[0].InitAsDescriptorTable(2, ranges, D3D12_SHADER_VISIBILITY_VERTEX);

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    GPU->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSig));


    {
        CD3DX12_SHADER_BYTECODE vsBytecode;
        CD3DX12_SHADER_BYTECODE psBytecode;

        {
#include "compiled_shaders\simple_vs.h"
            vsBytecode = CD3DX12_SHADER_BYTECODE(g_main, ARRAYSIZE(g_main));
        }

        {
#include "compiled_shaders\simple_ps.h"
            psBytecode = CD3DX12_SHADER_BYTECODE(g_main, ARRAYSIZE(g_main));
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

        psoDesc.InputLayout = VertexPosition::InputLayout;
        psoDesc.pRootSignature = RootSig;

        psoDesc.VS = vsBytecode;
        psoDesc.PS = psBytecode;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
        psoDesc.SampleDesc.Count = 1;

        if (FAILED(GPU->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO))))
        {
            throw std::runtime_error("failed to create pipeline state object!");
        }
    }

    {
        float aspect_ratio = m_deviceResources->GetScreenViewport().Width / m_deviceResources->GetScreenViewport().Height;

        VertexPosition vertices[] =
        {
            XMFLOAT3{ 0.0f, 0.25f * aspect_ratio, 0.0f },
            XMFLOAT3{ 0.25f, -0.25f * aspect_ratio, 0.0f },
            XMFLOAT3{ -0.25f, -0.25f * aspect_ratio, 0.0f }
        };

        UINT vbSize = sizeof(vertices);

        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
        GPU->CreateCommittedResource(&heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&VertexBuffer));

        void* GPUMem;
        CD3DX12_RANGE readRange(0, 0);
        VertexBuffer->Map(0, &readRange, &GPUMem);
        memcpy(GPUMem, vertices, sizeof(vertices));
        VertexBuffer->Unmap(0, nullptr);

        vbView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
        vbView.SizeInBytes = vbSize;
        vbView.StrideInBytes = sizeof(VertexPosition);
    }


    {
        float aspect_ratio = m_deviceResources->GetScreenViewport().Width / m_deviceResources->GetScreenViewport().Height;
        XMMATRIX View = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        XMMATRIX Projection = XMMatrixPerspectiveFovLH(XM_PI / 4, aspect_ratio, 0.1f, 10000.0f);

        view_projection vp{};
        vp.view = View;
        vp.projection = Projection;
        

        UINT cbSize = 256;
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
        GPU->CreateCommittedResource(&heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&CBViewProjection));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
        cbvDesc.BufferLocation = CBViewProjection->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = 256; //minimum constant buffer size.

        GPU->CreateConstantBufferView(&cbvDesc, Descriptors->GetCpuHandle(DescriptorIndex::ViewProjection));

        void* GPUMem;
        CD3DX12_RANGE readRange(0, 0);
        CBViewProjection->Map(0, &readRange, &GPUMem);
        memcpy(GPUMem, &vp, sizeof(view_projection));
        CBViewProjection->Unmap(0, nullptr);
    }


    {
        XMMATRIX World = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

        UINT cbSize = 256; //minimum constant buffer size.
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
        GPU->CreateCommittedResource(&heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&CBWorld));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
        cbvDesc.BufferLocation = CBWorld->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = 256; //minimum constant buffer size.

        GPU->CreateConstantBufferView(&cbvDesc, Descriptors->GetCpuHandle(DescriptorIndex::World));

        void* GPUMem;
        CD3DX12_RANGE readRange(0, 0);
        CBWorld->Map(0, &readRange, &GPUMem);
        memcpy(GPUMem, &World, sizeof(XMMATRIX));
        CBWorld->Unmap(0, nullptr);
    }
}

void Game::InitializeDescriptorHeap()
{
    Descriptors = std::make_unique<DescriptorHeap>(GPU, DescriptorIndex::Count);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
