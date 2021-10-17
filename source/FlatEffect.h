#pragma once
#include "BaseEffect.h"

class FlatEffect final : public BaseEffect
{
public:
	FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~FlatEffect();
	FlatEffect(const FlatEffect& other) = delete;
	FlatEffect& operator=(const FlatEffect& other) = delete;
	FlatEffect(FlatEffect&& other) = delete;
	FlatEffect& operator=(FlatEffect&& other) = delete;

	void SetTransparency(bool state) override;
	void SetCulling(Culling cull) override;
	void SetSampler(SampleMode technique) override;
};

