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

struct GPUHeapAllocation
{
    ID3D12Heap* pHeap;
    uint64      offset;
    uint64      size;

    GPUHeapAllocation()
    {

    }
    GPUHeapAllocation(ID3D12Heap* pHeap, uint64 offset, uint64 size)
    {
        this->pHeap = pHeap;
        this->offset = offset;
        this->size = size;
    }
};

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

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

    std::mutex& GetCopyEngineLock();
private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    ID3D12Device* GPU;

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    //input
    std::unique_ptr<DirectX::GamePad>  Controller;
    std::unique_ptr<DirectX::Keyboard> Keyboard;
    DirectX::Keyboard::State KeyboardState;
    DirectX::GamePad::State  GamepadState;
    void InitializeInput();
    //


    //------------------------------------------------------------------------------------------------------------------
    //gpu memory management
    //todo: replace with proper gpu memory manager
    ID3D12Heap* DynamicHeap;
    ID3D12Heap* StaticHeap;
    ID3D12Heap* TextureHeap;

    UINT64 StaticHeapIndex;
    inline void AdvanceStaticHeap(uint64 lastBufferSize)
    {
        if (lastBufferSize > D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
        {
            DynamicHeapIndex += lastBufferSize; //add last buffer size
            DynamicHeapIndex = AlignUp<uint64>(DynamicHeapIndex, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        }
        //last buffer placement size is not larger than single page of gpu memory
        //just align forward
        DynamicHeapIndex = AlignUp<uint64>(DynamicHeapIndex + 1, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
    }

    UINT64 DynamicHeapIndex;
    inline void AdvanceDynamicHeap(uint64 lastBufferSize)
    {
        if (lastBufferSize > D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
        {
            DynamicHeapIndex += lastBufferSize; //add last buffer size
            DynamicHeapIndex = AlignUp<uint64>(DynamicHeapIndex, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        }
        //last buffer placement size is not larger than single page of gpu memory
        //just align forward
        DynamicHeapIndex = AlignUp<uint64>(DynamicHeapIndex+1, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
    }

    UINT64 TextureHeapIndex;
    inline void AdvanceTextureHeapIndex(uint64 lastTextureSize)
    {
        if (lastTextureSize > D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
        {
            DynamicHeapIndex += lastTextureSize; //add last buffer size
            DynamicHeapIndex = AlignUp<uint64>(DynamicHeapIndex, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        }
        //last buffer placement size is not larger than single page of gpu memory
        //just align forward
        DynamicHeapIndex = AlignUp<uint64>(DynamicHeapIndex + 1, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
    }
    
    void InitializeHeaps(uint64 staticHeapSizeMB, uint64 dynamicHeapSizeMB, uint64 textureHeapSizeMB);
    //------------------------------------------------------------------------------------------------------------------
    StaticGeometryBuffer* GeoBuffer;
    GPUDescriptorHeap* SRVHeap;
    void InitializeDescriptorHeap();

    //------------------------------------------------------------------------------------------------------------------
    //copy engine
    Direct3DQueue* CopyQueue;
    GPUCommandAllocator* CopyCommandAllocator; //current limitation: only one thread can be creating copy commands
    ID3D12GraphicsCommandList* GetCopyCommandList();
    void InitializeCopyEngine();
    std::mutex copyEngineLock;
    //------------------------------------------------------------------------------------------------------------------

    thread_pool BackgroundPool;

    // Rendering loop timer.
    DX::StepTimer m_timer;
};