#pragma once
#include <vector>
#include "EMath.h"
#include "Camera.h"

class CameraManager final
{
public:
	static CameraManager* GetInstance() {
		if (m_Instance == nullptr) {
			m_Instance = new CameraManager();
		}
		return m_Instance;
	}
	virtual ~CameraManager();
	CameraManager(const CameraManager& other) = delete;
	CameraManager& operator=(const CameraManager& other) = delete;
	CameraManager(CameraManager&& other) = delete;
	CameraManager& operator=(CameraManager&& other) = delete;

	void AddNewCamera(Camera* camera);
	Camera* GetActiveCamera();
	void CycleTroughCameras();
	void RecalculateCameras();
private:
	CameraManager();

	//Variables
	static CameraManager* m_Instance;
	std::vector<Camera*> m_Cameras;
	Camera* m_ActiveCamera;
	int m_ActiveCameraId;
};

