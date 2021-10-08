#include "pch.h"
#include "Texture2D.h"
#include "LoaderHelpers.h"

Texture2D::Texture2D(ID3D12Device* pGPU)
{
	GPU = pGPU;
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

void* Texture2D::GetGPUStagingMemory()
{
	return stagingGPUMem;
}

void Texture2D::CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC vd{};
	GPU->CreateShaderResourceView(texture, &vd, descriptorHandle);
}