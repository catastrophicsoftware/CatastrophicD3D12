#include "pch.h"
#include "Texture2D.h"
#include "LoaderHelpers.h"
#include "ResourceUploadBatch.h"
#include "Game.h"

Texture2D::Texture2D(Game* pEngine)
{
	Engine = pEngine;
	GPU = Engine->GetGPUResources()->GetD3DDevice();
	hasSRV = false;
	hasUAV = false;
}

Texture2D::~Texture2D()
{
}

HRESULT Texture2D::Create(uint32 width, uint32 height, DXGI_FORMAT format)
{
	this->width = width;
	this->height = height;
	this->format = format;

	D3D12_RESOURCE_DESC td{};
	td.DepthOrArraySize = 1;
	td.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	td.MipLevels = 1;
	td.Width = width;
	td.Height = height;
	td.Format = format;
	td.SampleDesc.Count = 1;

	UINT64 textureSize = (DirectX::LoaderHelpers::BitsPerPixel(format) / 8) * (width * height);
	size = textureSize;
	state = D3D12_RESOURCE_STATE_COPY_DEST; //waiting for data to be loaded

	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

	if (!FAILED(GPU->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &td, state, nullptr, IID_PPV_ARGS(&texture))))
	{
		//create staging resource
		CD3DX12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize);
		if (!FAILED(GPU->CreateCommittedResource(&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&stagingDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer))))
		{
			CD3DX12_RANGE readRange(0, 0); //we will not be reading back from staging buffer

			if (FAILED(stagingBuffer->Map(0, &readRange, &stagingGPUMem)))
			{
				throw new std::runtime_error("failed to map staging buffer!");
			}
		}
		else
			throw new std::runtime_error("failed to create staging buffer for texture!");
	}
	else
		throw new std::runtime_error("failed to create texture2D!");
}

HRESULT Texture2D::LoadFromDDS(std::wstring fileName, ID3D12CommandQueue* pQueue)
{
	DirectX::ResourceUploadBatch tempUpload(GPU);

	tempUpload.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (SUCCEEDED(DirectX::CreateDDSTextureFromFile(GPU, tempUpload, fileName.c_str(), &texture, true)))
	{
		auto uploadTask = tempUpload.End(pQueue);
		uploadTask.wait(); //wait for gpu upload command to complete

		CreateSRV(Engine->GetGlobalSRVHeap());

		return S_OK;
	}
}

void Texture2D::Update(ID3D12GraphicsCommandList* pCopyCmdList, void* pTexData)
{
	D3D12_SUBRESOURCE_DATA texData{};
	texData.pData = pTexData;
	texData.RowPitch = width * (DirectX::LoaderHelpers::BitsPerPixel(format) / 8);
	texData.SlicePitch = texData.RowPitch * height;

	UpdateSubresources(pCopyCmdList, texture, stagingBuffer, 0, 0, 1, &texData);
}

void Texture2D::Update(ID3D12GraphicsCommandList* pCopyCmdList,void* pTexData,  D3D12_RESOURCE_STATES newState)
{
	D3D12_SUBRESOURCE_DATA texData{};
	texData.pData = pTexData;
	texData.RowPitch = width * (DirectX::LoaderHelpers::BitsPerPixel(format) / 8);
	texData.SlicePitch = texData.RowPitch * height;

	UpdateSubresources(pCopyCmdList, texture, stagingBuffer, 0, 0, 1, &texData);

	if (state != newState)
	{
		CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, state, newState, 1);
		pCopyCmdList->ResourceBarrier(1, &transition); //perform transition to new state
		state = newState;
	}
}

D3D12_RESOURCE_STATES Texture2D::GetResourceState() const
{
	return state;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture2D::GetSRV() const
{
	return srv.hGPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture2D::GetUAV() const
{
	return uav.hGPU;
}

void* Texture2D::GetGPUStagingMemory()
{
	return stagingGPUMem;
}

//creates a shader resource view
// pDescriptorHeap must be a GPUDescriptorHeap with available SRV descriptors
void Texture2D::CreateSRV(GPUDescriptorHeap* pDescriptorHeap)
{
	if (!hasSRV)
	{
		srv = pDescriptorHeap->GetUnusedDescriptor();

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{}; //currently unused

		viewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.Texture2D.MostDetailedMip = 0;
		viewDesc.Texture2D.MipLevels = -1;
		//TODO: improve SRV creation to not hardcore format and mip data

		GPU->CreateShaderResourceView(texture, &viewDesc, srv.hCPU);
		hasSRV = true;
	}
}

//creates an unordered access view
// pDescriptorHeap must be a GPUDescriptorHeap with available UAV descriptors
void Texture2D::CreateUAV(GPUDescriptorHeap* pDescriptorHeap)
{
	if (!hasUAV)
	{
		uav = pDescriptorHeap->GetUnusedDescriptor();
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		GPU->CreateUnorderedAccessView(texture, nullptr, &uavDesc, uav.hCPU);
		hasUAV = true;
	}
}