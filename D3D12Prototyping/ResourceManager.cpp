#include "pch.h"
#include "ResourceManager.h"

ResourceManager::ResourceManager(Game* pEngine)
{
	Engine = pEngine;
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::LoadMesh(std::string meshFile)
{

}

void ResourceManager::LoadTexture(std::string textureFile)
{
}

GPUMeshData* ResourceManager::GetGPUMesh(std::string name)
{
	return MeshResources[name];
}

void ResourceManager::RegisterMeshResource(GPUMeshData* meshData, std::string name)
{
	assert(!MeshResources.contains(name));

	MeshResources.insert(std::pair<std::string, GPUMeshData*>(name, meshData));
}

Texture2D* ResourceManager::GetTexture(std::string name)
{
	return TextureResources[name];
}

void ResourceManager::RegisterTextureresource(Texture2D* pTexture, std::string name)
{
	assert(!TextureResources.contains(name));

	TextureResources.insert(std::pair<std::string, Texture2D*>(name, pTexture));
}
