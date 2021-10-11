#pragma once
#include <d3d12.h>
#include "GPUCommandAllocator.h"
typedef UINT64 uint64;

class GPUQueue;

struct InflightGPUWork
{
    InflightGPUWork();
    InflightGPUWork(GPUQueue* pQueue, uint64 fenceVal);
    ~InflightGPUWork();

    GPUQueue* pGPUQueue; //10-10-2021 -- reference to queue will not be needed once queue management is improved. only a uint64 fence value will be required to 
    //know which queue to the fence is from
    uint64 fenceValue;

    void BlockUntilComplete(); //blocks current cpu thread until gpu work is complete
    bool IsComplete();
};

class GPUQueue
{
public:
    GPUQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType);
    ~GPUQueue();

    bool IsFenceComplete(uint64 fenceValue);
    void InsertWait(uint64 fenceValue);
    void InsertWaitForQueueFence(GPUQueue* otherQueue, uint64 fenceValue);
    void InsertWaitForQueue(GPUQueue* otherQueue);

    void WaitForFenceCPUBlocking(uint64 fenceValue);
    void Flush() { WaitForFenceCPUBlocking(mNextFenceValue - 1); }

    ID3D12CommandQueue* GetCommandQueue() { return mCommandQueue; }

    uint64 PollCurrentFenceValue();
    uint64 GetLastCompletedFence() { return mLastCompletedFenceValue; }
    uint64 GetNextFenceValue() { return mNextFenceValue; }
    ID3D12Fence* GetFence() { return mFence; }

    uint64 ExecuteCommandList(ID3D12CommandList* List);
    InflightCommandList ExecuteAndGetInflightHandle(ID3D12CommandList* List);
    InflightGPUWork Execute(ID3D12CommandList* List);

    void Destroy();
private:
    ID3D12CommandQueue* mCommandQueue;
    D3D12_COMMAND_LIST_TYPE mQueueType;

    std::mutex mFenceMutex;
    std::mutex mEventMutex;

    ID3D12Fence* mFence;
    uint64 mNextFenceValue;
    uint64 mLastCompletedFenceValue;
    HANDLE mFenceEventHandle;
};