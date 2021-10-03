#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

uint32 Mesh::VertexCount() const
{
	return NumVertices;
}

uint32 Mesh::IndexCount() const
{
	return NumIndices;
}

D3D12_PRIMITIVE_TOPOLOGY Mesh::Topology() const
{
	return topology;
}




HRESULT Mesh::Load(std::string MeshFile)
{
	Assimp::Importer Loader;

	const aiScene* AssimpScene = Loader.ReadFile(MeshFile, aiProcess_Triangulate);

	if (!AssimpScene || AssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !AssimpScene->mRootNode)
	{
		throw new std::exception("Error loading mesh");
		return E_FAIL;
	}
	else
	{
		ProcessAssimpMeshNode(AssimpScene->mRootNode, AssimpScene);

		IndexFormat = DXGI_FORMAT_R32_UINT;

		NumIndices = Indices.size();
		NumVertices = Vertices.size();
		VertexStride = sizeof(VertexPositionNormalTexture);
		
		topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		return S_OK;
	}

	return E_NOTIMPL;
}

void Mesh::SetGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	gpuAddress = gpuAddr;
}



void Mesh::ProcessAssimpMeshNode(aiNode * pNode, const aiScene * pScene)
{
	for (int i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* CurrentMesh = pScene->mMeshes[pNode->mMeshes[i]];

		for (int x = 0; x < CurrentMesh->mNumVertices; ++x)
		{
			XMFLOAT3 Position = XMFLOAT3(CurrentMesh->mVertices[x].x, CurrentMesh->mVertices[x].y, CurrentMesh->mVertices[x].z);
			XMFLOAT3 Normal = XMFLOAT3(CurrentMesh->mNormals[x].x, CurrentMesh->mNormals[x].y, CurrentMesh->mNormals[x].z);
			XMFLOAT2 Texcoord = XMFLOAT2(CurrentMesh->mTextureCoords[x]->x, CurrentMesh->mTextureCoords[x]->y);
			Vertices.push_back(VertexPositionNormalTexture(Position, Normal, Texcoord));
		}

		for (int y = 0; y < CurrentMesh->mNumFaces; ++y)
		{
			aiFace CurrentFace = CurrentMesh->mFaces[y];

			for (int j = 0; j < CurrentFace.mNumIndices; ++j)
			{
				Indices.push_back(CurrentFace.mIndices[j]);
			}
		}
	}

	for (int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessAssimpMeshNode(pNode->mChildren[i], pScene);
	}
}