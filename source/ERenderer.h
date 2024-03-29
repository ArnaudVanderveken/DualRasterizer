/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>

#include "ECamera.h"

#include "structs.h"
#include <vector>

struct SDL_Window;
struct SDL_Surface;

class Geometry;
class Triangle;
class Mesh;
class Texture;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow, Camera* pCamera);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render();
		void Update(float dt);

		void SwitchSampleFilter();
		void SwitchCullMode();
		void SwitchRenderMode();
		void ToggleRotating();
		void ToggleFireFX();

	private:
		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;

		bool m_IsInitialized;
		bool m_IsRotating;
		float m_Angle;
		float m_RotateSpeed;

		Camera* m_pCamera;

		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		float* m_DepthBuffer;
		uint32_t* m_pBackBufferPixels = nullptr;

		ComPtr<ID3D11Device> m_pDevice;
		ComPtr<ID3D11DeviceContext> m_pDeviceContext;
		ComPtr<IDXGIFactory> m_pDXGIFactory;
		ComPtr<IDXGISwapChain> m_pSwapChain;
		ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;
		ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
		ComPtr<ID3D11Resource> m_pRenderTargetBuffer;
		ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

		SampleMode m_SampleMode = SampleMode::point;
		CullMode m_CullMode = CullMode::backface;

		// Meshes
		//Vehicle
		unique_ptr<Mesh> m_pVehicle;
		unique_ptr<Texture> m_pDiffuseMap;
		unique_ptr<Texture> m_pNormalMap;
		unique_ptr<Texture> m_pSpecularMap;
		unique_ptr<Texture> m_pGlossinessMap;
		//FireFX
		bool m_ShowFireFX;
		unique_ptr<Mesh> m_pFireFX;
		unique_ptr<Texture> m_pFireFXDiffuse;

		// Sampling
		RasterMode m_RasterMode = RasterMode::hardware;

		// Member Functions
		void InitializeDirectX();
		void RenderTriangleMesh(Mesh* pMesh);
		void ResetDepthBuffer() const;
		void RenderTriangle(Triangle* pTriangle);
		void VertexShader(const std::vector<Vertex_Input>& inputVertices, std::vector<Vertex_Input>& outputVertices) const;
		Elite::RGBColor PixelShader(const RGBColor& diffuse, const RGBColor& specular, const RGBColor& ambient, float phongExponent, const FVector3& normal, const FVector3& viewDirection) const;
		Elite::IVector4 GetBoundingBox(const std::vector<Vertex_Input>& vertices) const;
		bool IsPointInTriangle(float w0, float w1, float w2) const;
	};
}

#endif //ELITE_RAYTRACING_RENDERER