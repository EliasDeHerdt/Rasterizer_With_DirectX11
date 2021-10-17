#pragma once
class Texture final
{
public:
	Texture(const std::string& path, ID3D11Device* pDevice);
	~Texture();
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;
	Texture(Texture&& other) = delete;
	Texture& operator=(Texture&& other) = delete;

	bool IsValid() const;
	Elite::RGBColor Sample(const Elite::FVector2& uv) const;
	ID3D11ShaderResourceView* GetTextureResourceView() const;
private:
	SDL_Surface* m_pTexture;
	ID3D11Texture2D* m_pGPUTexture;
	ID3D11ShaderResourceView* m_pTextureResourceView;
};

