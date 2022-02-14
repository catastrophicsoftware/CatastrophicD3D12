#include "pch.h"
#include "Game.h"

#include "VertexTypes.h"
#include <d3dcompiler.h>
#include <fstream>
#include "ForwardRenderer.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false) : BackgroundPool()
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

inline std::string GetAssetPath(std::string assetName)
{
    std::string baseAssetDirectory = "C:\\Users\\funkb\\source\\repos\\D3D12Prototyping\\Gaming.Desktop.x64\\Debug\\assets\\";
    return baseAssetDirectory + assetName;
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

    InitializeGPUMemory();
    InitializeGPUQueues();
    InitializeCopyEngine();
    InitializeStaticDescriptorHeaps(1024, 256, 32);
    InitializeStaticGeometryBuffer();



    //------------------------------------------------------------------------------------

    CubeModel = new Mesh();
    CubeModel->Load(GetAssetPath("building.obj"));

    /*GPUBuffer* vertexBuffer = new GPUBuffer(GPU);
    vertexBuffer->Create(sizeof(VertexPositionNormalTexture) * CubeModel->VertexCount(),D3D12_RESOURCE_STATE_GENERIC_READ, CENGINE_BUFFER_FLAGS::BUFFER_FLAG_NONE, true);
    
    auto pGPUMemory = vertexBuffer->Map();
    memcpy(pGPUMemory, CubeModel->GetVertexDataPointer(), sizeof(VertexPositionNormalTexture) * CubeModel->VertexCount());
    vertexBuffer->Unmap();

    GPUBuffer* indexBuffer = new GPUBuffer(GPU);
    indexBuffer->Create(sizeof(int) * CubeModel->IndexCount(), D3D12_RESOURCE_STATE_GENERIC_READ, BUFFER_FLAG_NONE, true);
    pGPUMemory = indexBuffer->Map();
    memcpy(pGPUMemory, CubeModel->GetIndexDataPointer(), sizeof(int) * CubeModel->IndexCount());
    indexBuffer->Unmap();*/

    auto vertexHandle = EngineStaticGeometry->WriteVertices(CubeModel->GetVertexDataPointer(), sizeof(VertexPositionNormalTexture), CubeModel->VertexCount());
    auto indexHandle = EngineStaticGeometry->WriteIndices(CubeModel->GetIndexDataPointer(), CubeModel->IndexCount());
    EngineStaticGeometry->Commit();


    CubeModel->CreateBufferViews(vertexHandle, indexHandle);

    Renderer = new ForwardRenderer(this);
    Renderer->InitializeRenderer();

    mainCamera = Camera();
    mainCamera.LookAt(XMVectorSet(0.0f, 0.0f, 20.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    mainCamera.SetLens(XMConvertToRadians(45.0f), (float)width / height, 0.1f, 10000.0f);
    mainCamera.UpdateViewMatrix();

    Renderer->UpdateViewProjection(XMMatrixTranspose(mainCamera.View()), XMMatrixTranspose(mainCamera.Proj()));


    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0f / 60.0f);
}

#pragma region Frame Update
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        GamepadState = Controller->GetState(0);
        Update(m_timer);
    });

    Render();
}

void Game::Update(DX::StepTimer const& timer)
{
#ifndef DISABLE_PIX_EVENTS
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update"); // See pch.h for info
#endif

    float elapsedTime = float(timer.GetElapsedSeconds());


    //movement
    if (GamepadState.thumbSticks.leftX > 0.1f)
        mainCamera.Strafe(0.2f);

    if (-0.1f > GamepadState.thumbSticks.leftX)
        mainCamera.Strafe(-0.2f);

    if (GamepadState.thumbSticks.leftY > 0.1f)
        mainCamera.Walk(0.2f);

    if (-0.1f > GamepadState.thumbSticks.leftY)
        mainCamera.Walk(-0.2f);

    if (GamepadState.thumbSticks.rightX > 0.1f)
        mainCamera.RotateY(0.02f);

    if (-0.1f > GamepadState.thumbSticks.rightX)
        mainCamera.RotateY(-0.02f);

    if (GamepadState.thumbSticks.rightY > 0.1f)
        mainCamera.Pitch(-0.02f);

    if (-0.1f > GamepadState.thumbSticks.rightY)
        mainCamera.Pitch(0.02f);


    mainCamera.UpdateViewMatrix();

    // TODO: Add your game logic here.
    elapsedTime;

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
void Game::Render()
{
    UINT fIndex = m_deviceResources->GetCurrentFrameIndex() % m_deviceResources->GetBackBufferCount();

    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
#ifndef DISABLE_PIX_EVENTS
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render"); // See pch.h for info
#endif
    //-----------------------------------------------------------------------------------------

    Renderer->BeginFrame(commandList, fIndex);
    Renderer->UpdateViewProjection(XMMatrixTranspose(mainCamera.View()), XMMatrixTranspose(mainCamera.Proj()));
    XMMATRIX worldTransform = XMMatrixIdentity() * XMMatrixTranslation(0.0f, 0.0f, 10.0f);

    Renderer->UpdateWorldTransform(XMMatrixTranspose(worldTransform));

    Renderer->Render(CubeModel);

    
    Renderer->EndFrame();


    //------------------------------------------------------------------------------------------

#ifndef DISABLE_PIX_EVENTS
    PIXEndEvent(commandList);
#endif

#ifndef DISABLE_PIX_EVENTS
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present"); // See pch.h for info
#endif
    m_deviceResources->Present();

#ifndef DISABLE_PIX_EVENTS
    PIXEndEvent(m_deviceResources->GetCommandQueue());
#endif

    //TODO: cleanup DISABLE_PIX_EVENTS
}

void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();

#ifndef DISABLE_PIX_EVENTS
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear"); // See pch.h for info
#endif

    // Clear the views.
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::DarkSlateBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

#ifndef DISABLE_PIX_EVENTS
    PIXEndEvent(commandList);
#endif
}
#pragma endregion

#pragma region Message Handlers
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
}

void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 1280;
    height = 720;
}

std::mutex& Game::GetCopyEngineLock()
{
    return copyEngineLock;
}


std::shared_ptr<DX::DeviceResources> Game::GetGPUResources() const
{
    return m_deviceResources;
}
#pragma endregion

#pragma region Direct3D Resources
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
}



void Game::InitializeStaticGeometryBuffer()
{
    EngineStaticGeometry = new StaticGeometryBuffer(GPU, CopyQueue);
    EngineStaticGeometry->Create(64);
}

void Game::InitializeInput()
{
    Controller = std::make_unique<GamePad>();
    Keyboard = std::make_unique<DirectX::Keyboard>();
}

void Game::InitializeGPUMemory()
{
    IDXGIAdapter1* pAdapter = nullptr;
    m_deviceResources->GetAdapter(&pAdapter);

    D3D12MA::ALLOCATOR_DESC desc{};
    desc.pAdapter = pAdapter;
    desc.pDevice = GPU;
    desc.Flags = D3D12MA::ALLOCATOR_FLAGS::ALLOCATOR_FLAG_NONE;
    desc.PreferredBlockSize = 0; //default of 64MB blocks

    if (FAILED(D3D12MA::CreateAllocator(&desc, &GPUMemory)))
    {
        throw new std::runtime_error("failed to create gpu memory allocator!");
    }

    const uint32 perFrameBufferSize = (1024 * 1024) * 16;
    for (int i = 0; i < m_deviceResources->GetBackBufferCount(); ++i)
    {
        LinearConstantBuffer* PerFrameBuffer = new LinearConstantBuffer(GPU, 16);
        
        PerFrameMemory.push_back(PerFrameBuffer);
    }
}

ID3D12Resource* Game::CreateGPUResource(D3D12_RESOURCE_DESC* resourceDesc, bool cpuWritable)
{
    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.HeapType = cpuWritable ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
    allocDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_NONE;
    
    ID3D12Resource* pNewResource = nullptr;

    D3D12_RESOURCE_STATES initialState = cpuWritable ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST;
    //generic-read for upload heap, otherwise copy_dest state for pending copy upload

    if (FAILED(GPUMemory->CreateResource(&allocDesc,
        resourceDesc,
        initialState,
        nullptr,
        nullptr,
        IID_PPV_ARGS(&pNewResource))))
    {
        throw std::runtime_error("failed to create resource!");
    }
}

void Game::InitializeStaticDescriptorHeaps(uint32 numSRVDescriptors, uint32 numRTVDescriptors, uint32 numSamplerDescriptors)
{
    SRVHeap = new GPUDescriptorHeap(GPU,numSRVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    RTVHeap = new GPUDescriptorHeap(GPU,numRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,false);
    SamplerHeap = new GPUDescriptorHeap(GPU, numSamplerDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

ID3D12GraphicsCommandList* Game::GetCopyCommandList()
{
    std::lock_guard<std::mutex> _lock(copyEngineLock);

    return CopyCommandAllocator->GetCommandList();
}

void Game::InitializeCopyEngine()
{
    CopyQueue = new GPUQueue(m_deviceResources->GetD3DDevice(), D3D12_COMMAND_LIST_TYPE_COPY);
    CopyCommandAllocator = new GPUCommandAllocator(m_deviceResources->GetD3DDevice(), D3D12_COMMAND_LIST_TYPE_COPY);
}

void Game::InitializeGPUQueues()
{
    ComputeQueue = new GPUQueue(GPU, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    ComputeCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_COMPUTE);

    GraphicsQueue = new GPUQueue(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
    GraphicsCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
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