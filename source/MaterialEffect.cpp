#include "pch.h"
#include "MaterialEffect.h"
MaterialEffect::MaterialEffect(ID3D11Device* pDevice, const std::wstring& assetFile, float lightIntensity, float shininess)
	: BaseEffect(pDevice, assetFile)
	, m_LightIntensity{ lightIntensity }
	, m_Shininess{ shininess }
	, m_pFloatPIVariable{ }
	, m_pFloatLightIntensityVariable{ }
	, m_pFloatShininessVariable{ }
	, m_pVecLightDirectionVariable{ }
	, m_pMatViewInverseVariable{ }
	, m_pNormalMapVariable{ }
	, m_pSpecularMapVariable{ }
	, m_pGlossinessMapVariable{ }
{
	m_Type = BaseEffect::EffectType::Material;

	//Constants
	m_pFloatPIVariable = m_pEffect->GetVariableByName("gPI")->AsScalar();
	if (!m_pFloatPIVariable->IsValid())
		std::wcout << L"FloatPIVariable not valid\n";

	m_pFloatLightIntensityVariable = m_pEffect->GetVariableByName("gLightIntensity")->AsScalar();
	if (!m_pFloatLightIntensityVariable->IsValid())
		std::wcout << L"FloatLightIntensityVariable not valid\n";

	m_pFloatShininessVariable = m_pEffect->GetVariableByName("gShininess")->AsScalar();
	if (!m_pFloatShininessVariable->IsValid())
		std::wcout << L"FloatShininessVariable not valid\n";

	m_pVecLightDirectionVariable = m_pEffect->GetVariableByName("gLightDirection")->AsVector();
	if (!m_pVecLightDirectionVariable->IsValid())
		std::wcout << L"VecLightDirectionVariable not valid\n";

	//Matrices
	m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if (!m_pMatViewInverseVariable->IsValid())
		std::wcout << L"MatViewInverseVariable not valid\n";

	//Textures
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::wcout << L"NormalMapVariable not valid\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::wcout << L"SpecularMapVariable not valid\n";

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
		std::wcout << L"GlossinessMapVariable not valid\n";

	//Set Constants value
	SetConstants();
}

MaterialEffect::~MaterialEffect()
{
}

void MaterialEffect::SetWorldMatrix(const Elite::FMatrix4& worldMatrix)
{
	if (!m_pMatWorldMatrixVariable->IsValid())
		return;

	const auto* data = &worldMatrix.data[0][0];
	m_pMatWorldMatrixVariable->SetMatrix(data);
}

void MaterialEffect::SetViewInverseMatrix(const Elite::FMatrix4& viewInverseMatrix)
{
	if (!m_pMatViewInverseVariable->IsValid())
		return;

	const auto* data = &viewInverseMatrix.data[0][0];
	m_pMatViewInverseVariable->SetMatrix(data);
}

void MaterialEffect::SetNormalMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pNormalMapVariable->IsValid())
		m_pNormalMapVariable->SetResource(pResourceView);
}

void MaterialEffect::SetSpecularMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pSpecularMapVariable->IsValid())
		m_pSpecularMapVariable->SetResource(pResourceView);
}

void MaterialEffect::SetGlossinessMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pGlossinessMapVariable->IsValid())
		m_pGlossinessMapVariable->SetResource(pResourceView);
}

float MaterialEffect::GetLightIntensity() const
{
	return m_LightIntensity;
}

float MaterialEffect::GetShininess() const
{
	return m_Shininess;
}

//This function does nothing for this class
void MaterialEffect::SetTransparency(bool state)
{
}

void MaterialEffect::SetCulling(Culling cull)
{
	m_CullMode = cull;
	HRESULT result;
	switch (m_CullMode)
	{
	case BaseEffect::Culling::Back:
		result = m_pRasterizer->SetRasterizerState(0, m_pBackCulling);
		if (FAILED(result))
			std::wcout << L"Set rasterizer is not valid\n";
		break;
	case BaseEffect::Culling::Front:
		result = m_pRasterizer->SetRasterizerState(0, m_pFrontCulling);
		if (FAILED(result))
			std::wcout << L"Set rasterizer is not valid\n";
		break;
	case BaseEffect::Culling::None:
		result = m_pRasterizer->SetRasterizerState(0, m_pNoCulling);
		if (FAILED(result))
			std::wcout << L"Set rasterizer is not valid\n";
		break;
	default:
		break;
	}
}

void MaterialEffect::SetSampler(SampleMode technique)
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

void MaterialEffect::SetConstants()
{
	if (m_pFloatPIVariable->IsValid())
		m_pFloatPIVariable->SetFloat(float(M_PI));

	//Defaulth of Matthieu is 7.f
	if (m_pFloatLightIntensityVariable->IsValid())
		m_pFloatLightIntensityVariable->SetFloat(m_LightIntensity);;

	//Defaulth of Matthieu is 25.f
	if (m_pFloatShininessVariable->IsValid())
		m_pFloatShininessVariable->SetFloat(m_Shininess);;

	if (m_pVecLightDirectionVariable->IsValid()) {

		Elite::FVector3 lightDirection = { 0.577f, -0.577f, 0.577f };

		const auto* data = &lightDirection.data[0];
		m_pVecLightDirectionVariable->SetFloatVector(data);
	}
}