#include "pch.h"
#include "CameraManager.h"

CameraManager* CameraManager::m_Instance = nullptr;

CameraManager::CameraManager()
	: m_ActiveCamera{ nullptr }
	, m_ActiveCameraId{ 0 }
{

}

CameraManager::~CameraManager()
{
	for (Camera* cam : m_Cameras)
		delete cam;
}

void CameraManager::AddNewCamera(Camera* camera)
{
	m_Cameras.push_back(camera);
	if (m_ActiveCamera == nullptr)
		m_ActiveCamera = m_Cameras[0];
}

Camera* CameraManager::GetActiveCamera()
{
	return m_ActiveCamera;
}

void CameraManager::CycleTroughCameras()
{
	if (m_ActiveCameraId + 1.f < m_Cameras.size())
		m_ActiveCamera = m_Cameras[++m_ActiveCameraId];
	else {
		m_ActiveCameraId = 0;
		m_ActiveCamera = m_Cameras[m_ActiveCameraId];
	}
}

//Force every camera to recalculate its coordinate system
void CameraManager::RecalculateCameras()
{
	for (Camera* cam : m_Cameras)
		cam->RecalculateCoordianteSystem();
}
