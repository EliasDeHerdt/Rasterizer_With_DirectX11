#include "pch.h"
#include "FlatEffect.h"

FlatEffect::FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	m_Type = BaseEffect::EffectType::Flat;
	m_CullMode = Culling::None;

	HRESULT result = m_pRasterizer->SetRasterizerState(0, m_pNoCulling);
	if (FAILED(result))
		throw std::runtime_error("Rasterizer could not be set.\n");

	result = m_pBlender->SetBlendState(0, m_pTransparencyOn);
	if (FAILED(result))
		throw std::runtime_error("Blend could not be set.\n");
}

FlatEffect::~FlatEffect()
{
}

void FlatEffect::SetTransparency(bool state)
{
	HRESULT result;
	if (state) {

		result = m_pBlender->SetBlendState(0, m_pTransparencyOn);
		if (FAILED(result))
			std::wcout << L"Set blend is not valid\n";
	}
	else {

		result = m_pBlender->SetBlendState(0, m_pTransparencyOff);
		if (FAILED(result))
			std::wcout << L"Set blend is not valid\n";
	}

}

//This function does nothing for this class
void FlatEffect::SetCulling(Culling cull)
{
}

void FlatEffect::SetSampler(SampleMode technique)
{
	switch (technique)
	{
	case BaseEffect::SampleMode::Point:
		m_pSampler->SetSampler(0, m_pPointSampler);
		if (!m_pSampler->IsValid())
			std::wcout << L"Set sampler is not valid\n";
		break;
	case BaseEffect::SampleMode::Linear:
		m_pSampler->SetSampler(0, m_pLinearSampler);
		if (!m_pTechnique->IsValid())
			std::wcout << L"Set sampler is not valid\n";
		break;
	case BaseEffect::SampleMode::Anisotropic:
		m_pSampler->SetSampler(0, m_pAnisotropicSampler);
		if (!m_pTechnique->IsValid())
			std::wcout << L"Set sampler is not valid\n";
		break;
	default:
		break;
	}
}
