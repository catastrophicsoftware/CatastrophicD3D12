#pragma once
#include "pch.h"
#include "GPUCommandQueue.h"


GPUQueue::GPUQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType)
{
    mQueueType = commandType;
    mCommandQueue = NULL;
    mFence = NULL;
    mNextFenceValue = ((uint64_t)mQueueType << 56) + 1;
    mLastCompletedFenceValue = ((uint64_t)mQueueType << 56);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = mQueueType;
    queueDesc.NodeMask = 0;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));

    mFence->Signal(mLastCompletedFenceValue);

    mFenceEventHandle = CreateEventEx(NULL, NULL, false, EVENT_ALL_ACCESS);
    assert(mFenceEventHandle != INVALID_HANDLE_VALUE);
}


GPUQueue::~GPUQueue()
{
    CloseHandle(mFenceEventHandle);

    mFence->Release();
    mFence = NULL;

    mCommandQueue->Release();
    mCommandQueue = NULL;
}


uint64 GPUQueue::PollCurrentFenceValue()
{
    mLastCompletedFenceValue = std::max<uint64>(mLastCompletedFenceValue, mFence->GetCompletedValue());

    return mLastCompletedFenceValue;
}

bool GPUQueue::IsFenceComplete(uint64 fenceValue)
{
    if (fenceValue > mLastCompletedFenceValue)
    {
        PollCurrentFenceValue();
    }

    return fenceValue <= mLastCompletedFenceValue;
}


void GPUQueue::InsertWait(uint64 fenceValue)
{
    mCommandQueue->Wait(mFence, fenceValue);
}

void GPUQueue::InsertWaitForQueueFence(GPUQueue* otherQueue, uint64 fenceValue)
{
    mCommandQueue->Wait(otherQueue->GetFence(), fenceValue);
}

void GPUQueue::InsertWaitForQueue(GPUQueue* otherQueue)
{
    mCommandQueue->Wait(otherQueue->GetFence(), otherQueue->GetNextFenceValue() - 1);
}

void GPUQueue::WaitForFenceCPUBlocking(uint64 fenceValue)
{
    if (IsFenceComplete(fenceValue))
    {
        return;
    }

    {
        LOCK_GUARD(mEventMutex);

        mFence->SetEventOnCompletion(fenceValue, mFenceEventHandle);
        WaitForSingleObjectEx(mFenceEventHandle, INFINITE, false);
        mLastCompletedFenceValue = fenceValue;
    }
}

uint64 GPUQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
    ((ID3D12GraphicsCommandList*)commandList)->Close();

    mCommandQueue->ExecuteCommandLists(1, &commandList);

    LOCK_GUARD(mFenceMutex);

    mCommandQueue->Signal(mFence, mNextFenceValue);

    return mNextFenceValue++;
}

InflightCommandList GPUQueue::ExecuteAndGetInflightHandle(ID3D12CommandList* List)
{
    uint64 value = ExecuteCommandList(List);
    InflightCommandList handle{List,this,value};
    return handle;
}

InflightGPUWork GPUQueue::Execute(ID3D12CommandList* List)
{
    InflightGPUWork workHandle{};
    workHandle.pGPUQueue = this;
    workHandle.fenceValue = ExecuteCommandList(List);

    return workHandle;
}

void GPUQueue::Destroy()
{
    Flush(); //flush queue

    mCommandQueue->Release();
    mFence->Release();
}

InflightGPUWork::InflightGPUWork()
{
    pGPUQueue = nullptr;
    fenceValue = 0;
}

InflightGPUWork::InflightGPUWork(GPUQueue* pQueue, uint64 fenceVal)
{
    this->pGPUQueue = pQueue;
    this->fenceValue = fenceVal;
}

InflightGPUWork::~InflightGPUWork()
{
}

void InflightGPUWork::BlockUntilComplete()
{
    pGPUQueue->WaitForFenceCPUBlocking(fenceValue);
}

bool InflightGPUWork::IsComplete()
{
    return pGPUQueue->GetLastCompletedFence() >= fenceValue;
}