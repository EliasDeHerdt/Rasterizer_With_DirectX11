#include "pch.h"
#include "Camera.h"
#include "SceneGraph.h"

Camera::Camera(const Elite::FPoint3& location, const Elite::FVector3& direction, float screenWidth, float screenHeight, float nearPlane, float farPlane, float FOV)
	: m_Location{ location }
	, m_Direction{ direction }
	, m_AspectRatio{ screenWidth / screenHeight }
	, m_NearPlane{ nearPlane }
	, m_FarPlane{ farPlane }
	, m_FOV{ tan(Elite::ToRadians(FOV) / 2) }
	, m_Right{ Elite::Cross(Elite::FVector3{ 0, 1, 0 }, direction) }
	, m_Up{ Elite::Cross(direction, m_Right) }
	, m_LookAtMatrix{}
	, m_ProjectionMatrix{}
{
	int renderMode = int(SceneGraph::GetInstance()->GetRenderMode());
	m_ProjectionMatrix = { 1 / (m_AspectRatio * m_FOV), 0	 , 0													 , 0
					 , 0						  , 1 / m_FOV, 0													 , 0
					 , 0						  , 0		 , m_FarPlane / ((m_FarPlane - m_NearPlane) * renderMode), (-(m_FarPlane * m_NearPlane) / ((m_FarPlane - m_NearPlane) * renderMode))* renderMode
					 , 0						  , 0		 , (float)renderMode									 , 0 };
	ReconstructONB();
}

const Elite::FPoint3& Camera::GetLocation() const
{
	return m_Location;
}

//Renamed GetLookAtMatrix() to match the slides
//Actually the inversed viewmatrix,
//but we use the inverse as the normal viewmatrix to not make the formulas any more confusing
const Elite::FMatrix4& Camera::GetViewMatrix() const
{
	return m_InverseLookAtMatrix;
}

//Actually the original viewmatrix, but named like this to not make the formulas any more confusing
const Elite::FMatrix4& Camera::GetInverseViewMatrix() const
{
	return m_LookAtMatrix;
}

const Elite::FMatrix4& Camera::GetProjectionMatrix() const
{
	return m_ProjectionMatrix;
}

const Elite::FVector3& Camera::GetViewDirection() const
{
	return m_Direction;
}

float Camera::GetNearPlane() const
{
	return m_NearPlane;
}

float Camera::GetFarPlane() const
{
	return m_FarPlane;
}

float Camera::GetFOV() const
{
	return m_FOV;
}

void Camera::MoveCamera(const Elite::FVector3& movement)
{
	float movementSlow{50};
	int renderMode = int(SceneGraph::GetInstance()->GetRenderMode());

	m_Location += (m_Right * movement.x / movementSlow);
	m_Location += (m_Up * movement.y / movementSlow);
	m_Location += (m_Direction * movement.z / movementSlow);
	ReconstructONB();
}

//something wrong here
void Camera::RotateCamera(float yaw, float pitch)
{
	int renderMode = int(SceneGraph::GetInstance()->GetRenderMode());

	m_Direction = Elite::FVector3{ Elite::MakeRotation(-pitch, m_Right) * Elite::MakeRotation(-yaw, m_Up) * m_Direction };
	ReconstructONB();
}

void Camera::RecalculateCoordianteSystem()
{
	int renderMode = int(SceneGraph::GetInstance()->GetRenderMode());
	m_ProjectionMatrix = { 1 / (m_AspectRatio * m_FOV), 0	 , 0													 , 0
					 , 0						  , 1 / m_FOV, 0													 , 0
					 , 0						  , 0		 , m_FarPlane / ((m_FarPlane - m_NearPlane) * renderMode), (-(m_FarPlane * m_NearPlane) / ((m_FarPlane - m_NearPlane) * renderMode)) * renderMode
					 , 0						  , 0		 , (float)renderMode									 , 0 };
	ReconstructONB();
}

//For directX we invert the X and Y of the forwardVector!! (so -x & -y)
void Camera::ReconstructONB()
{
	int renderMode = int(SceneGraph::GetInstance()->GetRenderMode());
	m_Right = Elite::GetNormalized(Elite::Cross(Elite::FVector3{ 0, 1, 0 }, m_Direction));
	m_Up = Elite::GetNormalized(Elite::Cross(m_Direction, m_Right));

	Elite::FVector3 forward{ -m_Direction.x * renderMode, -m_Direction.y * renderMode, m_Direction.z};

	Elite::FVector3 right = Elite::GetNormalized(Elite::Cross(Elite::FVector3{ 0, 1, 0 }, forward));
	Elite::FVector3 up = Elite::GetNormalized(Elite::Cross(forward, right));
	m_LookAtMatrix = { right.x ,up.x	,forward.x ,m_Location.x
					, right.y  ,up.y	,forward.y ,m_Location.y
					, right.z  ,up.z	,forward.z ,(-m_Location.z * renderMode)
					, 0		   ,0		, 0		   ,1 };
	m_InverseLookAtMatrix = Elite::Inverse(m_LookAtMatrix);
}
