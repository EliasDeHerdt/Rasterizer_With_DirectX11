#include "pch.h"
#include "EffectManager.h"

EffectManager* EffectManager::m_Instance = nullptr;

EffectManager::EffectManager()
	: m_Effects{}
	, m_CurrentTechnique{ BaseEffect::SampleMode::Point }
	, m_CurrentCull{ BaseEffect::Culling::Back }
	, m_Transparency{ true }
{
	PrintPixelShadingInformation();
	PrintObjectCullingInformation();
	PrintTransparencyInformation();
};

EffectManager::~EffectManager()
{
	for (auto& currentObject : m_Effects)
		delete currentObject.second;
}

void EffectManager::PrintPixelShadingInformation()
{
	std::cout << "Using Pixel Shading technique: ";
	switch (m_CurrentTechnique)
	{
	case BaseEffect::SampleMode::Point:
		std::cout << "Point\n";
		break;
	case BaseEffect::SampleMode::Linear:
		std::cout << "Linear\n";
		break;
	case BaseEffect::SampleMode::Anisotropic:
		std::cout << "Anisotropic\n";
		break;
	}
}

void EffectManager::PrintObjectCullingInformation()
{
	std::cout << "Using Cullmode: ";
	switch (m_CurrentCull)
	{
	case BaseEffect::Culling::Back:
		std::cout << "Back\n";
		break;
	case BaseEffect::Culling::Front:
		std::cout << "Front\n";
		break;
	case BaseEffect::Culling::None:
		std::cout << "None\n";
		break;
	}
}

void EffectManager::PrintTransparencyInformation()
{
	std::cout << "Transparency: ";
	if (m_Transparency)
		std::cout << "On\n";
	else 
		std::cout << "Off\n";
}

void EffectManager::ToggleObjectPixelShading()
{
	m_CurrentTechnique = BaseEffect::SampleMode((int(m_CurrentTechnique) + 1) % 3);

	PrintPixelShadingInformation();
	for (auto& currentObject : m_Effects)
		currentObject.second->SetSampler(m_CurrentTechnique);
}

void EffectManager::ToggleObjectCulling()
{
	m_CurrentCull = BaseEffect::Culling((int(m_CurrentCull) + 1) % 3);

	PrintObjectCullingInformation();
	for (auto& currentObject : m_Effects)
		currentObject.second->SetCulling(m_CurrentCull);
}

void EffectManager::ToggleObjectTransparency()
{
	m_Transparency = !m_Transparency;

	PrintTransparencyInformation();
	for (auto& currentObject : m_Effects)
		currentObject.second->SetTransparency(m_Transparency);
}

BaseEffect* EffectManager::GetEffect(std::string key)
{
	return m_Effects.at(key);
}

void EffectManager::AddEffect(std::string key, BaseEffect* effect)
{
	m_Effects.emplace(key, effect);
}
