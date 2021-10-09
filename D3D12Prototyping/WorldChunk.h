#pragma once
#include "pch.h"
#include "Texture2D.h"
#include "D3D12MemAlloc.h"
#include "SimpleMath.h"
#include "GPUDescriptorHeap.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define CHUNK_DIMENSION 256 //256x256 pixels per chunk
#define CHUNK_ELEMENT_COUNT (CHUNK_DIMENSION * CHUNK_DIMENSION)

//packs rgba value into uint32
inline uint32 PackPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (a << 24) + (r << 16) + (g << 8) + b;
}

//maps 2D array indexes to a 1 dimensional array
inline uint32 Map2D(uint32 x, uint32 y)
{
	return CHUNK_DIMENSION * x + y;
}

//returns random float between 0 and 1
inline float RandFloat()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

const uint32 dark_green = PackPixel(0, 102, 0, 255);
const uint32 light_green = PackPixel(0, 153, 0, 255);
const uint32 dark_brown = PackPixel(102, 51, 0, 255);
const uint32 light_brown = PackPixel(153, 76, 0, 255);
const uint32 magenta = PackPixel(255, 0, 255, 255);

class WorldChunk
{
public:
	WorldChunk(ID3D12Device* pGPU);
	~WorldChunk();

	void Initialize(uint32 worldIndexX, uint32 worldIndexY, D3D12MA::Allocator* Memory);

	void TransitionTextureState(ID3D12GraphicsCommandList* pCMD, D3D12_RESOURCE_STATES newState);
	void UpdateGPUTexture(ID3D12GraphicsCommandList* pCMD);
	void MarkGPUUpdateComplete();

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSRV() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureUAV() const;
	bool IsDirty() const;
	bool CPUDataReady() const;
	Vector2 GetPosition() const;
	XMUINT2 GetIndex() const;

	void createSRV(GPUDescriptorHeap* DescriptorHeap); //TODO: find better place / time to handle this

	bool ReadyForRendering() const;
	D3D12_RESOURCE_STATES GetTextureState() const;
private:
	ID3D12Device*  GPU;

	uint32* cpuChunkData;
	bool    cpuDataDirty;      //true if cpu-side data has changed and needs to be propagated to gpu memory
	bool    cpuDataReady;      //true if memory is allocated and available for cpu-side chunk data
	bool    gpuUpdatePending;  //true if an update has been recorded but has not yet been executed yet
	void    allocateCPUChunkData(); //allocates memory for cpu-side chunk data
	void    releaseCPUChunkData();  //de-allocated cpu-side chunk data memory

	ID3D12Resource* texture;
	ID3D12Resource* textureStaging;
	D3D12MA::Allocation* pTextureAlloc;
	D3D12MA::Allocation* pStagingAlloc;
	D3D12_RESOURCE_STATES textureState;
	GPUDescriptorHandle textureSRV;
	GPUDescriptorHandle textureUAV; //unused
	bool textureCreated;
	bool textureSRVCreated;
	void CreateTexture(D3D12MA::Allocator* GPUMemory);
	void ReleaseTexture();


	Vector2 Position; //position in 2D world space
	XMUINT2 WorldIndex; //index of this chunk IE 1,1 is at 0x0 through 256,256
};