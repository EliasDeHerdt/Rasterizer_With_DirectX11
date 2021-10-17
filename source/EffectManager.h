#pragma once
#include <map>
#include "MaterialEffect.h"
#include "FlatEffect.h"

class EffectManager final
{
public:
	static EffectManager* GetInstance() {
		if (m_Instance == nullptr) {
			m_Instance = new EffectManager();
		}
		return m_Instance;
	}
	virtual ~EffectManager();
	EffectManager(const EffectManager& other) = delete;
	EffectManager& operator=(const EffectManager& other) = delete;
	EffectManager(EffectManager&& other) = delete;
	EffectManager& operator=(EffectManager&& other) = delete;

	void PrintPixelShadingInformation();
	void PrintObjectCullingInformation();
	void PrintTransparencyInformation();

	void ToggleObjectPixelShading();
	void ToggleObjectCulling();
	void ToggleObjectTransparency();

	BaseEffect* GetEffect(std::string key);
	void AddEffect(std::string key, BaseEffect* effect);
private:
	EffectManager();

	//Variables
	static EffectManager* m_Instance;
	std::map<std::string, BaseEffect*> m_Effects;
	BaseEffect::SampleMode m_CurrentTechnique;
	BaseEffect::Culling m_CurrentCull;
	bool m_Transparency;
};

