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
#include "SpriteRenderer.h"
#include "Camera2D.h"

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

    LinearConstantBuffer* GetPerFrameBuffer(uint32 frameIndex) const;
private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    ID3D12Device* GPU;
    std::shared_ptr<DX::DeviceResources> m_deviceResources;

    std::mutex graphicsQueueMutex;
    GPUQueue* GraphicsQueue;
    GPUCommandAllocator* GraphicsCommandAllocator;
    void InitializeQueues();

    //------------------------------------------------------------------------------------------------------------------
    // game
    
    WorldChunk* TestChunk;
    SpriteRenderer* Renderer;

    void InitializeWorld();
    void RenderWorld(uint32 index);
    Camera2D camera;
    //------------------------------------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------------------------------------
    //input
    std::unique_ptr<DirectX::GamePad>  Controller;
    std::unique_ptr<DirectX::Keyboard> Keyboard;
    DirectX::Keyboard::State KeyboardState;
    DirectX::GamePad::State  GamepadState;
    void InitializeInput();
    //------------------------------------------------------------------------------------------------------------------


    //------------------------------------------------------------------------------------------------------------------
    //gpu memory management
    D3D12MA::Allocator* GPUMemory;
    void InitializeGPUMemory();
    ID3D12Resource* CreateGPUResource(D3D12_RESOURCE_DESC* resourceDesc, bool cpuWritable=false);

    std::vector<LinearConstantBuffer*> PerFrameMemory;
    //------------------------------------------------------------------------------------------------------------------
    //static descriptor management
    GPUDescriptorHeap* SRVHeap;     //srv heap for cvb/srv/uavs
    GPUDescriptorHeap* RTVHeap;     //srv heap for rtvs
    GPUDescriptorHeap* SamplerHeap; //srv heap for samplers

    void InitializeStaticDescriptorHeaps(uint32 numSRVDescriptors,uint32 numRTVDescriptors,uint32 numSamplerDescriptors);

    //------------------------------------------------------------------------------------------------------------------
    //copy engine
    GPUQueue* CopyQueue;
    GPUCommandAllocator* CopyCommandAllocator; //current limitation: only one thread can be creating copy commands
    ID3D12GraphicsCommandList* GetCopyCommandList();
    void InitializeCopyEngine();
    std::mutex copyEngineLock;

    GPUBuffer* UploadBuffer; //global 512MB upload buffer
    //------------------------------------------------------------------------------------------------------------------
    // graphics + compute queue management
    
    GPUQueue* ComputeQueue;
    GPUCommandAllocator* ComputeCommandAllocator;
    void InitializeGPUQueues();
    //------------------------------------------------------------------------------------------------------------------

    thread_pool BackgroundPool;

    // Rendering loop timer.
    DX::StepTimer m_timer;
};