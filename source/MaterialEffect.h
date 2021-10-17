#pragma once
#include "BaseEffect.h"

class MaterialEffect final : public BaseEffect
{
public:
	MaterialEffect(ID3D11Device* pDevice, const std::wstring& assetFile, float lightIntensity = 7.f, float shininess = 25.f) ;
	~MaterialEffect();
	MaterialEffect(const MaterialEffect& other) = delete;
	MaterialEffect& operator=(const MaterialEffect& other) = delete;
	MaterialEffect(MaterialEffect&& other) = delete;
	MaterialEffect& operator=(MaterialEffect&& other) = delete;

	void SetWorldMatrix(const Elite::FMatrix4& worldMatrix);
	void SetViewInverseMatrix(const Elite::FMatrix4& viewInverseMatrix);
	void SetNormalMap(ID3D11ShaderResourceView* pResourceView);
	void SetSpecularMap(ID3D11ShaderResourceView* pResourceView);
	void SetGlossinessMap(ID3D11ShaderResourceView* pResourceView);

	float GetLightIntensity() const override;
	float GetShininess() const override;
	void SetTransparency(bool state) override;
	void SetCulling(Culling cull) override;
	void SetSampler(SampleMode technique) override;
protected:
	float m_LightIntensity;
	float m_Shininess;

	//Everything below gets auto release when his parent (m_pEffect) gets released
	//Constants
	ID3DX11EffectScalarVariable* m_pFloatPIVariable;
	ID3DX11EffectScalarVariable* m_pFloatLightIntensityVariable;
	ID3DX11EffectScalarVariable* m_pFloatShininessVariable;

	ID3DX11EffectVectorVariable* m_pVecLightDirectionVariable;

	//Matrices
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable;

	//Textures
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
	void SetConstants();
	
};