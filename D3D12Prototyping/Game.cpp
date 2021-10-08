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
    InitializeDescriptorHeap();

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
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update"); // See pch.h for info

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


    //------------------------------------------------------------------------------------------
    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present"); // See pch.h for info
    m_deviceResources->Present();
    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

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

void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}

std::mutex& Game::GetCopyEngineLock()
{
    return copyEngineLock;
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
    // TODO: Initialize windows-size dependent objects here.
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


void Game::InitializeDescriptorHeap()
{
    SRVHeap = new GPUDescriptorHeap(GPU, 128, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    RTVHeap = new GPUDescriptorHeap(GPU, 128, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,false);
    SamplerHeap = new GPUDescriptorHeap(GPU, 128, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

ID3D12GraphicsCommandList* Game::GetCopyCommandList()
{
    std::lock_guard<std::mutex> _lock(copyEngineLock);

    return CopyCommandAllocator->GetCommandList();
}

void Game::InitializeCopyEngine()
{
    CopyQueue = new Direct3DQueue(m_deviceResources->GetD3DDevice(), D3D12_COMMAND_LIST_TYPE_COPY);
    CopyCommandAllocator = new GPUCommandAllocator(m_deviceResources->GetD3DDevice(), D3D12_COMMAND_LIST_TYPE_COPY);
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