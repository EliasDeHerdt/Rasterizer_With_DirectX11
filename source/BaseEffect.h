#pragma once
class BaseEffect
{
public:
	enum class EffectType {
		Material,
		Flat
	};

	enum class SampleMode {
		Point = 0,
		Linear,
		Anisotropic
	};

	enum class Culling {
		Back = 0,
		Front,
		None
	};

	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~BaseEffect();
	BaseEffect(const BaseEffect& other) = delete;
	BaseEffect& operator=(const BaseEffect& other) = delete;
	BaseEffect(BaseEffect&& other) = delete;
	BaseEffect& operator=(BaseEffect&& other) = delete;

	EffectType GetEffectType() const;
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;
	ID3DX11EffectMatrixVariable* GetWorldViewProjectionMatrix() const;
	void SetWorldViewProjectionMatrix(const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix, const Elite::FMatrix4& worldMatrix);
	void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView);
	virtual float GetLightIntensity() const;
	
	virtual float GetShininess() const;
	virtual Culling GetCullMode() const;
	//This function only works for certain effects
	virtual void SetTransparency(bool state) = 0;
	//This function only works for certain effects
	virtual void SetCulling(Culling cull) = 0;
	virtual void SetSampler(SampleMode technique) = 0;
protected:
	EffectType m_Type;
	Culling m_CullMode;
	ID3DX11Effect* m_pEffect;

	//Everything below gets auto release when his parent (m_pEffect) gets released
	ID3DX11EffectTechnique* m_pTechnique;

	//Variables
	ID3DX11EffectSamplerVariable* m_pSampler;
	ID3DX11EffectRasterizerVariable* m_pRasterizer;
	ID3DX11EffectBlendVariable* m_pBlender;

	//Samplers
	ID3D11SamplerState* m_pPointSampler;
	ID3D11SamplerState* m_pLinearSampler;
	ID3D11SamplerState* m_pAnisotropicSampler;

	//Rasterizers
	ID3D11RasterizerState* m_pBackCulling;
	ID3D11RasterizerState* m_pFrontCulling;
	ID3D11RasterizerState* m_pNoCulling;

	//Blenders
	ID3D11BlendState* m_pTransparencyOn;
	ID3D11BlendState* m_pTransparencyOff;

	//Matrices
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectMatrixVariable* m_pMatWorldMatrixVariable;

	//Textures
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

	void CreateSamplers(ID3D11Device* pDevice);
	void CreateRasterizers(ID3D11Device* pDevice);
	void CreateBlenders(ID3D11Device* pDevice);
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};

