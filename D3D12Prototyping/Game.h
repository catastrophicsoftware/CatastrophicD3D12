//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "GamePad.h"
#include "Keyboard.h"
#include "thread_pool.h"
#include "DescriptorHeap.h"
#include "LinearConstantBuffer.h"
#include "DirectXHelpers.h"
#include "Mesh.h"

using namespace DirectX;

struct view_projection
{
    XMMATRIX view;
    XMMATRIX projection;
};

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

private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    ID3D12Device* GPU;

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    std::unique_ptr<DirectX::GamePad>  Controller;
    std::unique_ptr<DirectX::Keyboard> Keyboard;
    DirectX::Keyboard::State KeyboardState;
    DirectX::GamePad::State  GamepadState;
    void InitializeInput();

    ID3D12Heap* DynamicHeap;
    ID3D12Heap* StaticHeap;
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
    void InitializeHeaps(uint64 staticHeapSizeMB, uint64 dynamicHeapSizeMB);

    ID3D12RootSignature* RootSig;
    ID3DBlob* vs;
    ID3DBlob* ps;
    ID3D12PipelineState* PSO;
    ID3D12Resource* VertexBuffer;
    ID3D12Resource* CBViewProjection;
    ID3D12Resource* CBWorld;
    ID3D12Resource* CBMaterial;
    void* pCBMaterialGPUMemory;
    D3D12_VERTEX_BUFFER_VIEW vbView;
    D3D12_GPU_VIRTUAL_ADDRESS cbMaterialMagenta;

    Mesh* testMesh;


    LinearConstantBuffer* linearBuffer;
    void InitializePipeline();

    enum DescriptorIndex
    {
        ViewProjection,
        Count
    };

    std::unique_ptr<DescriptorHeap> Descriptors;
    void InitializeDescriptorHeap();

    thread_pool BackgroundPool;
    // Rendering loop timer.
    DX::StepTimer m_timer;
};