#include "pch.h"
#include "SceneGraph.h"

SceneGraph* SceneGraph::m_Instance = nullptr;

SceneGraph::SceneGraph() 
	: m_Objects{}
	, m_CurrentRenderMode{ RenderMode::DirectX }
{
	PrintRenderModeInfo();
};

SceneGraph::~SceneGraph()
{
	for (Mesh* pObj: m_Objects)
	{
		delete pObj;
	}
}

void SceneGraph::Update(float deltaTime)
{
	for (Mesh* mesh : m_Objects)
		mesh->Update(deltaTime);
}

void SceneGraph::AddObjectToGraph(Mesh* object) {
	m_Objects.push_back(object);
}

const std::vector<Mesh*>& SceneGraph::GetObjects()
{
	return m_Objects;
}

void SceneGraph::ToggleRenderMode()
{
	m_CurrentRenderMode = RenderMode(int(m_CurrentRenderMode) * -1);
	PrintRenderModeInfo();
}

void SceneGraph::PrintRenderModeInfo() const
{
	std::cout << "Using Render Mode: ";
	switch (m_CurrentRenderMode)
	{
	case RenderMode::Rasterizer:
		std::cout << "Rasterizer\n";
		break;
	case RenderMode::DirectX:
		std::cout << "DirectX\n";
		break;
	default:
		break;
	}
}

const RenderMode SceneGraph::GetRenderMode() const
{
	return m_CurrentRenderMode;
}
