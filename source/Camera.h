#pragma once
#include "EMath.h"

class Camera final
{
public:
	Camera(const Elite::FPoint3& location, const Elite::FVector3& direction, float screenWidth, float screenHeight, float nearPlane = .1f, float farPlane = 100.f, float FOVAngle = 90.f);
	
	const Elite::FPoint3& GetLocation() const;
	const Elite::FMatrix4& GetViewMatrix() const;
	const Elite::FMatrix4& GetInverseViewMatrix() const;
	const Elite::FMatrix4& GetProjectionMatrix() const;
	const Elite::FVector3& GetViewDirection() const;
	float GetNearPlane() const;
	float GetFarPlane() const;
	float GetFOV() const;

	void MoveCamera(const Elite::FVector3& movement);
	void RotateCamera( float pitch, float yaw);
	void RecalculateCoordianteSystem();
private:
	Elite::FPoint3 m_Location;
	Elite::FVector3 m_Direction;
	float m_AspectRatio;
	float m_NearPlane;
	float m_FarPlane;
	float m_FOV;

	Elite::FVector3 m_Right;
	Elite::FVector3 m_Up;
	Elite::FMatrix4 m_LookAtMatrix;
	Elite::FMatrix4 m_InverseLookAtMatrix;
	Elite::FMatrix4 m_ProjectionMatrix;

	void ReconstructONB();
};

