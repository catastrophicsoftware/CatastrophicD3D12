#pragma once
#include <map>
#include <string>

class GPUMeshData;
class Texture2D;
class Game;

class ResourceManager
{
public:
	ResourceManager(Game* pEngine);
	~ResourceManager();

	void LoadMesh(std::string meshFile);
	void LoadTexture(std::string textureFile);

	GPUMeshData* GetGPUMesh(std::string name);
	void RegisterMeshResource(GPUMeshData* meshData, std::string name);

	Texture2D* GetTexture(std::string name);
	void RegisterTextureresource(Texture2D* pTexture, std::string name);
private:
	Game* Engine;

	std::map<std::string, GPUMeshData*> MeshResources;
	std::map<std::string, Texture2D*> TextureResources;
};