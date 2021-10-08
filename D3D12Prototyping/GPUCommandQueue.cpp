#pragma once
#include "pch.h"
#include "GPUCommandQueue.h"


Direct3DQueue::Direct3DQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType)
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


Direct3DQueue::~Direct3DQueue()
{
    CloseHandle(mFenceEventHandle);

    mFence->Release();
    mFence = NULL;

    mCommandQueue->Release();
    mCommandQueue = NULL;
}


uint64 Direct3DQueue::PollCurrentFenceValue()
{
    mLastCompletedFenceValue = std::max<uint64>(mLastCompletedFenceValue, mFence->GetCompletedValue());

    return mLastCompletedFenceValue;
}

bool Direct3DQueue::IsFenceComplete(uint64 fenceValue)
{
    if (fenceValue > mLastCompletedFenceValue)
    {
        PollCurrentFenceValue();
    }

    return fenceValue <= mLastCompletedFenceValue;
}


void Direct3DQueue::InsertWait(uint64 fenceValue)
{
    mCommandQueue->Wait(mFence, fenceValue);
}

void Direct3DQueue::InsertWaitForQueueFence(Direct3DQueue* otherQueue, uint64 fenceValue)
{
    mCommandQueue->Wait(otherQueue->GetFence(), fenceValue);
}

void Direct3DQueue::InsertWaitForQueue(Direct3DQueue* otherQueue)
{
    mCommandQueue->Wait(otherQueue->GetFence(), otherQueue->GetNextFenceValue() - 1);
}

void Direct3DQueue::WaitForFenceCPUBlocking(uint64 fenceValue)
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

uint64 Direct3DQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
    ((ID3D12GraphicsCommandList*)commandList)->Close();

    mCommandQueue->ExecuteCommandLists(1, &commandList);

    LOCK_GUARD(mFenceMutex);

    mCommandQueue->Signal(mFence, mNextFenceValue);

    return mNextFenceValue++;
}

InflightCommandList Direct3DQueue::ExecuteAndGetInflightHandle(ID3D12CommandList* List)
{
    uint64 value = ExecuteCommandList(List);
    InflightCommandList handle{List,this,value};
    return handle;
}

void Direct3DQueue::Destroy()
{
    WaitForIdle(); //flush queue

    mCommandQueue->Release();
    mFence->Release();
}