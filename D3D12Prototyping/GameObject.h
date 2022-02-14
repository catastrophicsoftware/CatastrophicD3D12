#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class IGameObject
{
public:
	IGameObject();
	~IGameObject();

	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;


	void SetPosition(XMFLOAT3 position);
	void SetUpdateState(bool shouldUpdate);
	void SetVisibilityState(bool shouldRender);

	void UpdateTransform();
	XMMATRIX GetTransform() const;
	XMVECTOR GetPosition() const;
	XMFLOAT3 GetPositionFloat3() const;

	bool IsUpdating() const;
	bool IsRendering() const;
protected:
	XMMATRIX Transform;
	XMVECTOR Position;

	bool ShouldUpdate;
	bool ShouldRender;
};