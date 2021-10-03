#pragma once
#include "pch.h"

using namespace DirectX;

class Camera2D
{
public:
	Camera2D();
	~Camera2D();

	void SetViewport(D3D12_VIEWPORT Viewport);

	void Update();

	XMMATRIX GetTransform() const;

	XMFLOAT2 GetPosition() const;

	void SetPosition(XMFLOAT2 Position);

	XMFLOAT4 GetViewArea() const;
private:
	float Zoom;
	float Rotation;

	XMFLOAT2 Position;

	XMMATRIX Transform;

	XMFLOAT2 CameraViewAreaMin;
	XMFLOAT2 CameraViewAreaMax;

	D3D12_VIEWPORT CameraViewport;
};