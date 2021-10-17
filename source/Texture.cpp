#include "pch.h"
#include "Texture.h"
#include <SDL_image.h>

Texture::Texture(const std::string& path, ID3D11Device* pDevice)
	: m_pTexture{ nullptr }
	, m_pGPUTexture{ nullptr }
	, m_pTextureResourceView{ nullptr }
{
	if (path.empty())
		return;

	m_pTexture = IMG_Load(path.c_str());

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = m_pTexture->w;
	desc.Height = m_pTexture->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_pTexture->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pTexture->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pTexture->h * m_pTexture->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pGPUTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pGPUTexture, &SRVDesc, &m_pTextureResourceView);
	
	
}

Texture::~Texture()
{
	SDL_FreeSurface(m_pTexture);
	if (m_pTextureResourceView)
		m_pTextureResourceView->Release();
	if (m_pGPUTexture)
		m_pGPUTexture->Release();
}

bool Texture::IsValid() const
{
	return (m_pTexture != nullptr);
}

Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{
	Uint8 r, g, b;
	Elite::IVector2 newUv = { int(m_pTexture->w * uv.x), int(m_pTexture->h * uv.y) };

	uint32_t pixel = *((uint32_t*)m_pTexture->pixels + newUv.y * (size_t)m_pTexture->w + newUv.x);

	SDL_GetRGB(pixel, m_pTexture->format, &r, &g, &b);
	return Elite::RGBColor{ float(r), float(g) ,float(b) } / 255.f;
}
ID3D11ShaderResourceView* Texture::GetTextureResourceView() const
{
	return m_pTextureResourceView;
}
