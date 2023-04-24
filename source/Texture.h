#pragma once

class Texture final
{
public:
	Texture(const std::string& filePath, ID3D11Device* pDevice);
	~Texture();

	Texture(const Texture& other) noexcept = delete;
	Texture(Texture&& other) noexcept = delete;
	Texture& operator=(const Texture& other) noexcept = delete;
	Texture& operator=(Texture&& other) noexcept = delete;

	[[nodiscard]] ID3D11ShaderResourceView* GetResourceView() const;
	[[nodiscard]] ID3D11Texture2D* GetTexture() const;

	[[nodiscard]] Elite::RGBColor Sample(const Elite::FVector2& uv) const;
	[[nodiscard]] Uint32 PixelToIndex(const Elite::FVector2& uv) const;

private:
	ComPtr<ID3D11Texture2D> m_pTexture;
	ComPtr<ID3D11ShaderResourceView> m_pTextureResourceView;
	SDL_Surface* m_pSurface;
};

