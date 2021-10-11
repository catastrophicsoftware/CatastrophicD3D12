//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "VertexTypes.h"
#include <d3dcompiler.h>
#include <fstream>

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
    InitializeCopyEngine();
    InitializeStaticDescriptorHeaps(1024, 256, 32);
    InitializeQueues();

    InitializeWorld();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
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

    // TODO: Add your game logic here.
    elapsedTime;

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
void Game::Render()
{
    UINT fIndex = m_deviceResources->GetCurrentFrameIndex() % m_deviceResources->GetBackBufferCount();
    PerFrameMemory[fIndex]->Reset();

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

    RenderWorld(fIndex);

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
LinearConstantBuffer* Game::GetPerFrameBuffer(uint32 frameIndex) const
{
    assert(frameIndex <= (m_deviceResources->GetBackBufferCount() - 1));
    return PerFrameMemory[frameIndex];
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

void Game::InitializeQueues()
{
    GraphicsQueue = new GPUQueue(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
    GraphicsCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Game::InitializeWorld()
{
    TestChunk = new WorldChunk(GPU);
    TestChunk->Initialize(0, 0, GPUMemory);

    DX::DeviceResources* pEngine = m_deviceResources.get();
    Renderer = new SpriteRenderer(GPU, CopyQueue, SRVHeap, pEngine);
    Renderer->Initialize(m_deviceResources->GetBackBufferCount());

    auto copyCMD = GraphicsCommandAllocator->GetCommandList();
    TestChunk->UpdateGPUTexture(copyCMD);
    TestChunk->createSRV(SRVHeap);

    GraphicsQueue->WaitForFenceCPUBlocking(GraphicsQueue->ExecuteCommandList(copyCMD)); //submit and wait for texture copy work

    camera.SetPosition(XMFLOAT2(0.0f, 0.0f));
    camera.SetViewport(m_deviceResources->GetScreenViewport());
    camera.Update();
}

void Game::RenderWorld(uint32 index)
{
    Matrix cameraTransform = camera.GetTransform();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    Renderer->SetViewportAndScissor(m_deviceResources->GetScreenViewport(),
        m_deviceResources->GetScissorRect());

    Renderer->BeginRenderPass(index, cameraTransform, m_deviceResources->GetCommandList());

    Renderer->RenderSprite(TestChunk->GetTextureSRV(), Vector2(0.0f, 0.0f));

    Renderer->EndRenderPass();

    //m_deviceResources->GetCommandQueue()->Wait(spriteRenderWork.pGPUQueue->GetFence(), spriteRenderWork.fenceValue);
    ////insert gpu graphics pipeline halt until sprite rendering work is complete
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

    UploadBuffer = new GPUBuffer(GPU);
    uint64 uploadBufferSize = (1024 * 1024) * 512;
    UploadBuffer->Create(uploadBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, BUFFER_FLAG_LIFETIME_MAP, true);
}

void Game::InitializeGPUQueues()
{
    ComputeQueue = new GPUQueue(GPU, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    ComputeCommandAllocator = new GPUCommandAllocator(GPU, D3D12_COMMAND_LIST_TYPE_COMPUTE);
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