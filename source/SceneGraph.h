#pragma once
#include <vector>
#include "Mesh.h"
#include "Structs.h"

class SceneGraph final
{
public:
	static SceneGraph* GetInstance() {
		if (m_Instance == nullptr) {
			m_Instance = new SceneGraph();
		}
		return m_Instance;
	}
	~SceneGraph();
	SceneGraph(const SceneGraph& other) = delete;
	SceneGraph& operator=(const SceneGraph& other) = delete;
	SceneGraph(SceneGraph&& other) = delete;
	SceneGraph& operator=(SceneGraph&& other) = delete;

	void Update(float deltaTime);
	void AddObjectToGraph(Mesh* object);
	const std::vector<Mesh*>& GetObjects();
	const RenderMode GetRenderMode() const;
	void ToggleRenderMode();
	void PrintRenderModeInfo() const;
private:
	SceneGraph();

	//Variables
	static SceneGraph* m_Instance;
	std::vector<Mesh*> m_Objects;
	RenderMode m_CurrentRenderMode;
};