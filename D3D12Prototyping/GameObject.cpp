#include "pch.h"
#include "GameObject.h"

IGameObject::IGameObject()
{
}

IGameObject::~IGameObject()
{
}

void IGameObject::SetPosition(XMFLOAT3 position)
{
	Position = XMLoadFloat3(&position);
}

void IGameObject::SetUpdateState(bool shouldUpdate)
{
	ShouldUpdate = shouldUpdate;
}

void IGameObject::SetVisibilityState(bool shouldRender)
{
	ShouldRender = shouldRender;
}

void IGameObject::UpdateTransform()
{
}

XMMATRIX IGameObject::GetTransform() const
{
	return XMMATRIX();
}

XMVECTOR IGameObject::GetPosition() const
{
	return Position;
}

XMFLOAT3 IGameObject::GetPositionFloat3() const
{
	XMFLOAT3 outPosition = XMFLOAT3();
	XMStoreFloat3(&outPosition, Position);
	return outPosition;
}

bool IGameObject::IsUpdating() const
{
	return ShouldUpdate;
}

bool IGameObject::IsRendering() const
{
	return ShouldRender;
}