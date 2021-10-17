#include "pch.h"
#include "BaseEffect.h"
#include <sstream>

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: m_Type{ }
	, m_pEffect{ }
	, m_pTechnique{ }
	, m_pSampler{ }
	, m_pRasterizer{ }
	, m_pBlender{ }
	, m_pPointSampler{ }
	, m_pLinearSampler{ }
	, m_pAnisotropicSampler{ }
	, m_pBackCulling{ }
	, m_pFrontCulling{ }
	, m_pNoCulling{ }
	, m_pTransparencyOn{ }
	, m_pTransparencyOff{ }
	, m_pMatWorldViewProjVariable{ }
	, m_pMatWorldMatrixVariable{ }
	, m_pDiffuseMapVariable{ }
{
	m_pEffect = LoadEffect(pDevice, assetFile);

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";

	m_pSampler = m_pEffect->GetVariableByName("gSampler")->AsSampler();
	if (!m_pSampler->IsValid())
		std::wcout << L"Sampler not valid\n";

	m_pRasterizer = m_pEffect->GetVariableByName("gRasterizer")->AsRasterizer();
	if (!m_pRasterizer->IsValid())
		std::wcout << L"Rasterizer not valid\n";

	m_pBlender = m_pEffect->GetVariableByName("gBlend")->AsBlend();
	if (!m_pBlender->IsValid())
		std::wcout << L"Blend not valid\n";

	//Matrices
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::wcout << L"WorldViewProjectionMatrix not valid\n";

	m_pMatWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldMatrixVariable->IsValid())
		std::wcout << L"MatWorldMatrixVariable not valid\n";

	//Textures
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"DiffuseMapVariable not valid\n";

	CreateSamplers(pDevice);
	CreateRasterizers(pDevice);
	CreateBlenders(pDevice);
}

BaseEffect::~BaseEffect()
{
	if (m_pEffect)
		m_pEffect->Release();
	if(m_pPointSampler)
		m_pPointSampler->Release();
	if (m_pLinearSampler)
		m_pLinearSampler->Release();
	if (m_pAnisotropicSampler)
		m_pAnisotropicSampler->Release();
	if (m_pBackCulling)
		m_pBackCulling->Release();
	if (m_pFrontCulling)
		m_pFrontCulling->Release();
	if (m_pNoCulling)
		m_pNoCulling->Release();
	if (m_pTransparencyOn)
		m_pTransparencyOn->Release();
	if (m_pTransparencyOff)
		m_pTransparencyOff->Release();
}

BaseEffect::EffectType BaseEffect::GetEffectType() const
{
	return m_Type;
}

ID3DX11Effect* BaseEffect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* BaseEffect::GetTechnique() const
{
	return m_pTechnique;
}

ID3DX11EffectMatrixVariable* BaseEffect::GetWorldViewProjectionMatrix() const
{
	return m_pMatWorldViewProjVariable;
}

void BaseEffect::SetWorldViewProjectionMatrix(const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix, const Elite::FMatrix4& worldMatrix)
{
	if (!m_pMatWorldViewProjVariable->IsValid())
		return;

	Elite::FMatrix4 WorldViewProjectionMatrix = projectionMatrix * viewMatrix * worldMatrix;
	const auto* data = &WorldViewProjectionMatrix.data[0][0];

	m_pMatWorldViewProjVariable->SetMatrix(data);
}

void BaseEffect::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pDiffuseMapVariable->IsValid())
		m_pDiffuseMapVariable->SetResource(pResourceView);
}

//Returns light intensity. If no value is available, returns -1
float BaseEffect::GetLightIntensity() const
{
	return -1.f;
}

//Returns shininess. If no shininess is available, returns -1
float BaseEffect::GetShininess() const
{
	return -1.f;
}

BaseEffect::Culling BaseEffect::GetCullMode() const
{
	return m_CullMode;
}

void BaseEffect::CreateSamplers(ID3D11Device* pDevice)
{
	auto data = Elite::FPoint4(0.0f, 0.0f, 1.0f, 1.0f).data[0];

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; // Wrap or Mirror or Clamp or Border
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; // Wrap or Mirror or Clamp or Border
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; // Wrap or Mirror or Clamp or Border
	samplerDesc.BorderColor[3] = data;

	HRESULT result = pDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);
	if (FAILED(result))
		throw std::runtime_error("Point sampler could not be made.\n");

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = pDevice->CreateSamplerState(&samplerDesc, &m_pLinearSampler);
	if (FAILED(result))
		throw std::runtime_error("Linear sampler could not be made.\n");

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	result = pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropicSampler);
	if (FAILED(result))
		throw std::runtime_error("Anisotropic sampler could not be made.\n");

	result = m_pSampler->SetSampler(0, m_pPointSampler);
	if (FAILED(result))
		throw std::runtime_error("Sampler could not be set.\n");
}

void BaseEffect::CreateRasterizers(ID3D11Device* pDevice)
{
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	//Define that whenever the indices are placed in a counter clockwise manner,
	//it should be seen as a front-face
	rasterizerDesc.FrontCounterClockwise = true;

	HRESULT result = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pBackCulling);
	if (FAILED(result))
		throw std::runtime_error("Back cull rasterizer could not be made.\n");

	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pFrontCulling);
	if (FAILED(result))
		throw std::runtime_error("Front cull rasterizer could not be made.\n");

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pNoCulling);
	if (FAILED(result))
		throw std::runtime_error("No cull rasterizer could not be made.\n");

	result = m_pRasterizer->SetRasterizerState(0, m_pBackCulling);
	if (FAILED(result))
		throw std::runtime_error("Rasterizer could not be set.\n");
}

void BaseEffect::CreateBlenders(ID3D11Device* pDevice)
{
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT result = pDevice->CreateBlendState(&blendDesc, &m_pTransparencyOn);
	if (FAILED(result))
		throw std::runtime_error("Transparency on blend could not be made.\n");

	blendDesc.RenderTarget[0].BlendEnable = false;
	result = pDevice->CreateBlendState(&blendDesc, &m_pTransparencyOff);
	if (FAILED(result))
		throw std::runtime_error("Transparency off blend could not be made.\n");

	result = m_pBlender->SetBlendState(0, m_pTransparencyOff);
	if (FAILED(result))
		throw std::runtime_error("Blend could not be set.\n");
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result)) {

		if (pErrorBlob != nullptr) {

			char* pErrors = (char*)pErrorBlob->GetBufferPointer();

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else {

			std::wstringstream ss;
			ss << "Effectloader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}
