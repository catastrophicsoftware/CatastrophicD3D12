#include "pch.h"
#include "Camera2D.h"

Camera2D::Camera2D()
{
	Position = XMFLOAT2(0, 0);
	Zoom = 1.0f;
	Rotation = 0.0f;
}


Camera2D::~Camera2D()
{
}

void Camera2D::SetViewport(D3D12_VIEWPORT Viewport)
{
	CameraViewport = Viewport;
	CameraViewAreaMin = XMFLOAT2();
	CameraViewAreaMax = XMFLOAT2();
}

void Camera2D::Update()
{
	CameraViewAreaMin.x = Position.x - CameraViewport.Width / 2;
	CameraViewAreaMin.y = Position.y - CameraViewport.Height / 2;
	CameraViewAreaMax.x = Position.x + CameraViewport.Width / 2;
	CameraViewAreaMax.y = Position.y + CameraViewport.Height / 2;

	Transform = XMMatrixTranslation(-Position.x, -Position.y, 0.0f) *
		XMMatrixRotationZ(Rotation) *
		XMMatrixScaling(Zoom, Zoom, 1) *
		XMMatrixTranslation(CameraViewport.Width * 0.5f, CameraViewport.Height * 0.5f, 0.0f);
}

XMMATRIX Camera2D::GetTransform() const
{
	return Transform;
}

XMFLOAT2 Camera2D::GetPosition() const
{
	return Position;
}

void Camera2D::SetPosition(XMFLOAT2 Position)
{
	this->Position = Position;
}

XMFLOAT4 Camera2D::GetViewArea() const
{
	return XMFLOAT4(CameraViewAreaMin.x,
		CameraViewAreaMin.y,
		CameraViewAreaMax.x,
		CameraViewAreaMax.y);
}