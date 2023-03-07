#pragma once

class Texture
{
public:
	Texture(const std::string& filePath, ID3D11Device* pDevice);
	~Texture();

	ID3D11ShaderResourceView* GetResourceView() const;
	ID3D11Texture2D* GetTexture() const;

	Elite::RGBColor Sample(const Elite::FVector2& uv);
	Uint32 PixelToIndex(const Elite::FVector2& uv) const;

private:
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pTextureResourceView;
	SDL_Surface* m_pSurface;
};

