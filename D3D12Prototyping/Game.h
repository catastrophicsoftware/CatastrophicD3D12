//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "GamePad.h"
#include "Keyboard.h"
#include "thread_pool.h"
#include "GPUDescriptorHeap.h"
#include "LinearConstantBuffer.h"
#include "DirectXHelpers.h"
#include "Mesh.h"
#include "GPUCommandAllocator.h"
#include "GPUCommandQueue.h"
#include "Texture2D.h"
#include "StaticGeometryBuffer.h"
#include "D3D12MemAlloc.h"
#include "WorldChunk.h"

using namespace DirectX;

// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    void Initialize(HWND window, int width, int height);

    void Tick();

    void OnDeviceLost() override;
    void OnDeviceRestored() override;
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
    void GetDefaultSize( int& width, int& height ) const noexcept;

    std::mutex& GetCopyEngineLock();
private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    ID3D12Device* GPU;
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    std::mutex graphicsQueueMutex;
    GPUQueue* GraphicsQueue;
    GPUCommandAllocator* GraphicsCommandAllocator;
    void InitializeQueues();

    //-----------------------------------------------------
    // game
    
    WorldChunk* TestChunk;

    void InitializeWorld();
    void RenderWorld(ID3D12GraphicsCommandList* pCMD);
    //-----------------------------------------------------

    //input------------------------------------------------
    std::unique_ptr<DirectX::GamePad>  Controller;
    std::unique_ptr<DirectX::Keyboard> Keyboard;
    DirectX::Keyboard::State KeyboardState;
    DirectX::GamePad::State  GamepadState;
    void InitializeInput();
    //-----------------------------------------------------


    //------------------------------------------------------------------------------------------------------------------
    //gpu memory management
    D3D12MA::Allocator* GPUMemory;
    void InitializeGPUMemory();
    ID3D12Resource* CreateGPUResource(D3D12_RESOURCE_DESC* resourceDesc, bool cpuWritable=false);
    //------------------------------------------------------------------------------------------------------------------
    //static descriptor management
    GPUDescriptorHeap* SRVHeap;     //srv heap for cvb/srv/uavs
    GPUDescriptorHeap* RTVHeap;     //srv heap for rtvs
    GPUDescriptorHeap* SamplerHeap; //srv heap for samplers

    void InitializeStaticDescriptorHeaps();
    //------------------------------------------------------------------------------------------------------------------
    //copy engine
    GPUQueue* CopyQueue;
    GPUCommandAllocator* CopyCommandAllocator; //current limitation: only one thread can be creating copy commands
    ID3D12GraphicsCommandList* GetCopyCommandList();
    void InitializeCopyEngine();
    std::mutex copyEngineLock;
    //------------------------------------------------------------------------------------------------------------------

    thread_pool BackgroundPool;

    // Rendering loop timer.
    DX::StepTimer m_timer;
};