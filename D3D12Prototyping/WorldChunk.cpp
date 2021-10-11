#include "pch.h"
#include "WorldChunk.h"
#include "LoaderHelpers.h"

WorldChunk::WorldChunk(ID3D12Device* pGPU)
{
	cpuDataReady = false;
	cpuDataDirty = true;
	gpuUpdatePending = false;
	GPU = pGPU;
}

WorldChunk::~WorldChunk()
{
}

void WorldChunk::Initialize(uint32 worldIndexX, uint32 worldIndexY, D3D12MA::Allocator* Memory)
{
	WorldIndex = XMUINT2(worldIndexX, worldIndexY);
	Position = Vector2(256 * worldIndexX, 256 * worldIndexY);

	allocateCPUChunkData();
	CreateTexture(Memory);
}

void WorldChunk::TransitionTextureState(ID3D12GraphicsCommandList* pCMD, D3D12_RESOURCE_STATES newState)
{
	if (textureState == newState)
		return; //no transition needed

	CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, textureState, newState, 0);
	pCMD->ResourceBarrier(1, &transition);
}

void WorldChunk::UpdateGPUTexture(ID3D12GraphicsCommandList* pCMD)
{
	if (cpuDataDirty && cpuDataReady)
	{
		uint64 bytesPerPixel = DirectX::LoaderHelpers::BitsPerPixel(DXGI_FORMAT_R8G8B8A8_UINT) / 8;
		D3D12_SUBRESOURCE_DATA data{};
		data.pData = cpuChunkData;
		data.RowPitch   = CHUNK_DIMENSION * bytesPerPixel;
		data.SlicePitch = data.RowPitch * CHUNK_DIMENSION;

		UpdateSubresources(pCMD,
			texture,
			textureStaging,
			0, 0, 1,
			&data); //record copy command

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture, textureState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		pCMD->ResourceBarrier(1, &barrier); //transition to pixel shader resource after being filled with contents
		textureState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;


		gpuUpdatePending = true;
		cpuDataDirty = false;
		//Warning! command is recorded but has not yet begun execution
		//TODO: handle in-flight gpu commands
	}
}

void WorldChunk::MarkGPUUpdateComplete()
{
	if (gpuUpdatePending)
		gpuUpdatePending = false;
}

D3D12_GPU_DESCRIPTOR_HANDLE WorldChunk::GetTextureSRV() const
{
	return textureSRV.hGPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE WorldChunk::GetTextureUAV() const
{
	return D3D12_GPU_DESCRIPTOR_HANDLE(); //unused
}

bool WorldChunk::IsDirty() const
{
	return cpuDataDirty;
}

bool WorldChunk::CPUDataReady() const
{
	return cpuDataReady;
}

Vector2 WorldChunk::GetPosition() const
{
	return Position;
}

XMUINT2 WorldChunk::GetIndex() const
{
	return WorldIndex;
}

void WorldChunk::allocateCPUChunkData()
{
	srand(GetTickCount());
	if (!cpuDataReady)
	{
		cpuChunkData = new uint32[CHUNK_DIMENSION * CHUNK_DIMENSION];

		for (int i = 0; i < CHUNK_ELEMENT_COUNT; ++i)
		{
			float r = RandFloat();
			if (r > 0.5f)
				cpuChunkData[i] = light_green;
			else
				cpuChunkData[i] = dark_brown;
		}

		cpuDataReady = true;
		cpuDataDirty = true; //set if not already set
	}
}

void WorldChunk::releaseCPUChunkData()
{
	if (cpuDataReady)
	{
		delete[] cpuChunkData;
		cpuChunkData = nullptr;
		cpuDataReady = false;
	}
}

void WorldChunk::createSRV(GPUDescriptorHeap* DescriptorHeap)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Texture2D.MipLevels = 1;
	viewDesc.Texture2D.MostDetailedMip = 0;

	textureSRV = DescriptorHeap->GetUnusedDescriptor();

	GPU->CreateShaderResourceView(texture, &viewDesc, textureSRV.hCPU);
}

bool WorldChunk::ReadyForRendering() const
{
	return (gpuUpdatePending == false && textureState == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_RESOURCE_STATES WorldChunk::GetTextureState() const
{
	return textureState;
}

void WorldChunk::CreateTexture(D3D12MA::Allocator* GPUMemory)
{
	CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UINT,
		CHUNK_DIMENSION,
		CHUNK_DIMENSION,
		1,
		1);

	textureState = D3D12_RESOURCE_STATE_COPY_DEST; //waiting for initial data

	D3D12MA::ALLOCATION_DESC allocDesc{};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	if (FAILED(GPUMemory->CreateResource(&allocDesc,
		&texDesc,
		textureState,
		nullptr,
		&pTextureAlloc,
		IID_PPV_ARGS(&texture))))
	{
		throw std::runtime_error("failed to create chunk texture!");
	}
	else
	{
		uint32 bytesPerPixel = DirectX::LoaderHelpers::BitsPerPixel(DXGI_FORMAT_R8G8B8A8_UINT) / 8;
		uint32 stagingBufferSize = bytesPerPixel * (CHUNK_DIMENSION * CHUNK_DIMENSION);
		CD3DX12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(stagingBufferSize);
		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		D3D12MA::Allocation* pStagingAlloc;
		if (FAILED(GPUMemory->CreateResource(&allocDesc,
			&stagingDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&pStagingAlloc,
			IID_PPV_ARGS(&textureStaging)))) //TODO: look into not creating a staging resource per chunk and use a single large staging buffer instead
		{
			throw std::runtime_error("failed to create chunk staging buffer!");
		}

		textureCreated = true; //texture and staging resource exist, mark texture as created
	}
}

void WorldChunk::ReleaseTexture()
{
	if (textureCreated)
	{
		pTextureAlloc->Release();
		texture->Release();

		pStagingAlloc->Release();
		textureStaging->Release();

		textureCreated = false;
	}
}