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
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    std::unique_ptr<DirectX::GamePad>  Controller;
    std::unique_ptr<DirectX::Keyboard> Keyboard;
    DirectX::Keyboard::State KeyboardState;
    DirectX::GamePad::State  GamepadState;
    void InitializeInput();

    ID3D12RootSignature* RootSig;
    ID3DBlob* vs;
    ID3DBlob* ps;
    ID3D12PipelineState* PSO;
    ID3D12Resource* VertexBuffer;
    ID3D12Resource* CBViewProjection;
    ID3D12Resource* CBWorld;
    D3D12_VERTEX_BUFFER_VIEW vbView;
    void InitializePipeline();


    enum DescriptorIndex
    {
        ViewProjection,
        World,
        Count
    };

    std::unique_ptr<DescriptorHeap> Descriptors;
    void InitializeDescriptorHeap();

    thread_pool BackgroundPool;
    // Rendering loop timer.
    DX::StepTimer m_timer;
};
