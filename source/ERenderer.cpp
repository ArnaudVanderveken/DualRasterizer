#include "pch.h"

//Project includes
#include "ERenderer.h"

//My includes
#include "Texture.h"
#include "Triangle.h"
#include "Mesh.h"


Elite::Renderer::Renderer(SDL_Window * pWindow, Camera* pCamera)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
	, m_IsRotating{ true }
	, m_Angle{}
	, m_RotateSpeed{ -ToRadians(45.f) }
	, m_pCamera{ pCamera }
	, m_ShowFireFX{ true }
{
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_DepthBuffer = new float[width * height]{};
	ResetDepthBuffer();
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);

	InitializeDirectX();
	
	// Vehicle
	m_pVehicle = make_unique<Mesh>(m_pDevice.Get(), "Resources/vehicle.obj", FVector3(0, 0, 50));
	m_pDiffuseMap = make_unique<Texture>("Resources/vehicle_diffuse.png", m_pDevice.Get());
	m_pNormalMap = make_unique<Texture>("Resources/vehicle_normal.png", m_pDevice.Get());
	m_pSpecularMap = make_unique<Texture>("Resources/vehicle_specular.png", m_pDevice.Get());
	m_pGlossinessMap = make_unique<Texture>("Resources/vehicle_gloss.png", m_pDevice.Get());

	m_pVehicle->SetDiffuseMap(m_pDiffuseMap->GetResourceView());
	m_pVehicle->SetNormalMap(m_pNormalMap->GetResourceView());
	m_pVehicle->SetSpecularMap(m_pSpecularMap->GetResourceView());
	m_pVehicle->SetGlossinessMap(m_pGlossinessMap->GetResourceView());

	// FireFX
	m_pFireFX = make_unique<Mesh>(m_pDevice.Get(), "Resources/fireFX.obj", FVector3(0,0,50), true);
	m_pFireFXDiffuse = make_unique<Texture>("Resources/fireFX_diffuse.png", m_pDevice.Get());

	m_pFireFX->SetDiffuseMap(m_pFireFXDiffuse->GetResourceView());
}

Elite::Renderer::~Renderer()
{
	// SoftwareRasterizer
	delete[] m_DepthBuffer;

}

void Elite::Renderer::Render()
{
	if (m_RasterMode == RasterMode::hardware)
	{
		if (!m_IsInitialized)
			return;

		//Clear Buffers
		const RGBColor clearColor = RGBColor(0.1f, 0.1f, 0.1f);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Initialize Variables
		const FVector3 vehiclePos = m_pVehicle->GetPosition();

		FMatrix4 world = MakeTranslation(vehiclePos);
		world *= static_cast<FMatrix4>(MakeRotationY(m_Angle));

		const FMatrix4 worldViewPort = m_pCamera->GetWorldViewProjection();
		const FMatrix4 viewInv = m_pCamera->GetViewToWorld();

		// Render
		m_pCamera->SetWorldMatrix(world);
		m_pVehicle->SetWorldViewMatrix(reinterpret_cast<const float*>(worldViewPort.data));
		m_pVehicle->SetWorldMatrix(reinterpret_cast<float*>(world.data));
		m_pVehicle->SetViewInverseMatrix(reinterpret_cast<const float*>(viewInv.data));
		m_pVehicle->Render(m_pDeviceContext.Get());

		if (m_ShowFireFX)
		{
			m_pFireFX->SetWorldViewMatrix(reinterpret_cast<const float*>(worldViewPort.data));
			m_pFireFX->SetWorldMatrix(reinterpret_cast<float*>(world.data));
			m_pFireFX->SetViewInverseMatrix(reinterpret_cast<const float*>(viewInv.data));
			m_pFireFX->Render(m_pDeviceContext.Get());
		}

		//Present
		m_pSwapChain->Present(0, 0);
	}
	else 
	{
		SDL_LockSurface(m_pBackBuffer);

		ResetDepthBuffer();

		// Reset to black
		SDL_FillRect(m_pBackBuffer, nullptr, 0xFF1A1A1A);

		RenderTriangleMesh(m_pVehicle.get());
		//No Booster render in software rasterizer

		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
}

void Elite::Renderer::Update(float dt)
{
	if (m_IsRotating) m_Angle += dt * m_RotateSpeed;
}

void Elite::Renderer::SwitchSampleFilter()
{
	m_SampleMode = SampleMode((int(m_SampleMode) + 1) % int(SampleMode::SIZE));

	switch (m_SampleMode)
	{
	case SampleMode::point:
		std::cout << "POINT Sampling." << std::endl;
		break;

	case SampleMode::linear:
		std::cout << "LINEAR Sampling." << std::endl;
		break;

	case SampleMode::anisotropic:
		std::cout << "ANISOTROPIC Sampling." << std::endl;
		break;

	default:
		std::cout << "Invalid SampleMode" << std::endl;
		break;
	}

	m_pVehicle->SetTextureSamplingState(m_SampleMode);
	m_pFireFX->SetTextureSamplingState(m_SampleMode);
}

void Elite::Renderer::SwitchCullMode()
{
	m_CullMode = CullMode((int(m_CullMode) + 1) % int(CullMode::SIZE));

	switch (m_CullMode)
	{
	case CullMode::backface:
		std::cout << "BACKFACE culling." << std::endl;
		break;

	case CullMode::frontface:
		std::cout << "FRONTFACE culling." << std::endl;
		break;

	case CullMode::none:
		std::cout << "NO culling." << std::endl;
		break;

	default:
		std::cout << "Invalid CullMode" << std::endl;
		break;
	}

	m_pVehicle->SetCullMode(m_CullMode);
	m_pFireFX->SetCullMode(m_CullMode);
}

void Elite::Renderer::SwitchRenderMode()
{
	m_RasterMode = RasterMode((int(m_RasterMode) + 1) % int(RasterMode::SIZE));

	switch (m_RasterMode)
	{
	case RasterMode::hardware:
		std::cout << "HARDWARE rasterization." << std::endl;
		break;

	case RasterMode::software:
		std::cout << "SOFTWARE rasterization." << std::endl;
		break;

	default:
		std::cout << "Invalid RasterMode" << std::endl;
		break;
	}
}

void Elite::Renderer::ToggleRotating()
{
	m_IsRotating = !m_IsRotating;
	if (m_IsRotating)
		std::cout << "Rotation ENABLED.\n";
	else
		std::cout << "Rotation DISABLED.\n";
}

void Elite::Renderer::ToggleFireFX()
{
	m_ShowFireFX = !m_ShowFireFX;
	if (m_ShowFireFX)
		std::cout << "FireFX ENABLED.\n";
	else
		std::cout << "FireFX DISABLED.\n";
}

void Elite::Renderer::InitializeDirectX()
{
	//Initialize DirectX pipeline
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
	#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
	#endif
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result)) 
	{
		std::cout << "Error while creating DirectX Device!" << std::endl;
		return;
	}

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), &m_pDXGIFactory);
	if (FAILED(result))
	{
		std::cout << "Error while creating the Factory!" << std::endl;
		return;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	result = m_pDXGIFactory->CreateSwapChain(m_pDevice.Get(), &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
	{
		std::cout << "Error while creating the Swap Chain!" << std::endl;
		return;
	}

	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
	{
		std::cout << "Error while creating DepthStencil" << std::endl;
		return;
	}
	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
	{
		std::cout << "Error while creating DepthStencilView" << std::endl;
		return;
	}

	// Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &m_pRenderTargetBuffer);
	if (FAILED(result))
	{
		std::cout << "Error while creating RenderTargetBuffer" << std::endl;
		return;
	}
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer.Get(), 0, &m_pRenderTargetView);
	if (FAILED(result))
	{
		std::cout << "Error while creating RenderTargetView" << std::endl;
		return;
	}

	// Bind the views to the output merger stage
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	m_IsInitialized = true;
	std::cout << "DirectX is ready!" << std::endl;
}

void Elite::Renderer::RenderTriangleMesh(Mesh* pMesh)
{
	std::vector<Vertex_Input> vertices{ pMesh->GetVertexBuffer() };
	const std::vector<uint32_t> indexes{ pMesh->GetIndexBuffer() };
	for (size_t i{}; i < indexes.size() - 2; i += 3)
	{
		pMesh->SetTemplateVertices({ vertices[indexes[i]], vertices[indexes[i + 1]], vertices[indexes[i + 2]] });
		RenderTriangle(pMesh->GetTriangle());
	}
}

void Elite::Renderer::RenderTriangle(Triangle* pTriangle)
{
	std::vector<Vertex_Input> transformedVertices;

	// Transform vertices to view space
	VertexShader(pTriangle->GetColoredVertices(), transformedVertices);

	// Frustum Culling
	for (const Vertex_Input& vertex : transformedVertices)
	{
		if (vertex.Position.x > 1 || vertex.Position.x < -1) return;
		if (vertex.Position.y > 1 || vertex.Position.y < -1) return;
		if (vertex.Position.z > 1 || vertex.Position.z < 0) return;
	}

	// To Screen Space
	for (Vertex_Input& vertex : transformedVertices)
	{
		vertex.Position.x = (1 + vertex.Position.x) / 2 * m_Width;
		vertex.Position.y = (1 - vertex.Position.y) / 2 * m_Height;
	}

	// Bounding Box
	// x = minX
	// y = maxX
	// z = minY
	// w = maxY
	IVector4 boundingBox{ GetBoundingBox(transformedVertices) };

	// Loop over all the pixels in the box
	for (int r = boundingBox.z; r < boundingBox.w; ++r)
	{
		for (int c = boundingBox.x; c < boundingBox.y; ++c)
		{
			RGBColor color{ 0,0,0 };

			FPoint2 pixelPos{ static_cast<float>(c) + 0.5f, static_cast<float>(r) + 0.5f }; // + 0.5f for center of pixel
			FPoint2 v0{ transformedVertices[0].Position.xy };
			FPoint2 v1{ transformedVertices[1].Position.xy };
			FPoint2 v2{ transformedVertices[2].Position.xy };

			FPoint2 screenPoint{ pixelPos.x, pixelPos.y };

			float w0 = Cross(v2 - v1, screenPoint - v1);
			float w1 = Cross(v0 - v2, screenPoint - v2);
			float w2 = Cross(v1 - v0, screenPoint - v0);

			if (IsPointInTriangle(w0, w1, w2))
			{
				float totalArea = w0 + w1 + w2;
				w0 /= totalArea;
				w1 /= totalArea;
				w2 /= totalArea;

				float zDepth{ 1 / (1 / transformedVertices[0].Position.z * w0 + 1 / transformedVertices[1].Position.z * w1 + 1 / transformedVertices[2].Position.z * w2) };
				float wInterpolated{ 1 / (1 / transformedVertices[0].Position.w * w0 + 1 / transformedVertices[1].Position.w * w1 + 1 / transformedVertices[2].Position.w * w2) };

				if (zDepth < m_DepthBuffer[c + r * m_Width])
				{
					m_DepthBuffer[c + r * m_Width] = zDepth;

					// Pixel data interpolations
					FVector2 interpolatedUV{ (transformedVertices[0].UV / transformedVertices[0].Position.w * w0 + transformedVertices[1].UV / transformedVertices[1].Position.w * w1 + transformedVertices[2].UV / transformedVertices[2].Position.w * w2) * wInterpolated };
					FVector3 interpolatedNormal{ (transformedVertices[0].Normal / transformedVertices[0].Position.w * w0 + transformedVertices[1].Normal / transformedVertices[1].Position.w * w1 + transformedVertices[2].Normal / transformedVertices[2].Position.w * w2) * wInterpolated };
					interpolatedNormal = GetNormalized(interpolatedNormal);

					FVector3 interpolatedTangent{ (transformedVertices[0].Tangent / transformedVertices[0].Position.w * w0 + transformedVertices[1].Tangent / transformedVertices[1].Position.w * w1 + transformedVertices[2].Tangent / transformedVertices[2].Position.w * w2) * wInterpolated };
					interpolatedTangent = GetNormalized(interpolatedTangent);

					FMatrix3 tangentSpaceAxis{ interpolatedTangent, Cross(interpolatedNormal, interpolatedTangent), interpolatedNormal };
					RGBColor normalSample{ m_pNormalMap->Sample(interpolatedUV) };
					FVector3 trueNormal{ tangentSpaceAxis * FVector3{ 2 * normalSample.r - 1, 2 * normalSample.g - 1, 2 * normalSample.b - 1 } };

					FVector3 interpolatedViewDirection{ (transformedVertices[0].viewDirection / pTriangle->GetColoredVertices()[0].Position.w * w0 + transformedVertices[1].viewDirection / pTriangle->GetColoredVertices()[1].Position.w * w1 + transformedVertices[2].viewDirection / pTriangle->GetColoredVertices()[2].Position.w * w2) * wInterpolated };
					interpolatedViewDirection = GetNormalized(interpolatedViewDirection);

					color = PixelShader(m_pDiffuseMap->Sample(interpolatedUV), m_pSpecularMap->Sample(interpolatedUV), RGBColor{ 0.025f, 0.025f, 0.025f }, m_pGlossinessMap->Sample(interpolatedUV).r, trueNormal, interpolatedViewDirection);

					//Fill the pixels - pixel access demo
					m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
																		static_cast<uint8_t>(color.r * 255),
																		static_cast<uint8_t>(color.g * 255),
																		static_cast<uint8_t>(color.b * 255));
				}
			}
		}
	}
}

// Function that transforms the vertices from the mesh in world space into screen space
void Elite::Renderer::VertexShader(const std::vector<Vertex_Input>& inputVertices, std::vector<Vertex_Input>& outputVertices) const
{
	//outputVertices = inputVertices;
	outputVertices.resize(inputVertices.size());
	for (size_t i = 0; i < inputVertices.size(); i++)
	{
		auto rotMatrix{ Elite::MakeRotationY(m_Angle) };
		outputVertices[i].Position = Elite::FPoint4(rotMatrix * inputVertices[i].Position.xyz + m_pVehicle->GetPosition()); // World transformation
		outputVertices[i].Tangent = (rotMatrix * inputVertices[i].Tangent);
		outputVertices[i].Normal = (rotMatrix * inputVertices[i].Normal);
		outputVertices[i].UV = inputVertices[i].UV;
		outputVertices[i].viewDirection = m_pCamera->GetPosition() - FVector3(outputVertices[i].Position.xyz);

		outputVertices[i].Position = m_pCamera->GetProjectionMatrix() * m_pCamera->GetWorldToView() * outputVertices[i].Position;
		outputVertices[i].Position.x /= outputVertices[i].Position.w;
		outputVertices[i].Position.y /= outputVertices[i].Position.w;
		outputVertices[i].Position.z /= outputVertices[i].Position.w;
	}
}

Elite::RGBColor Elite::Renderer::PixelShader(const Elite::RGBColor& diffuse, const Elite::RGBColor& specular, const Elite::RGBColor& ambient, float phongExponent, const Elite::FVector3& normal, const Elite::FVector3& viewDirection) const
{
	constexpr float shininess{ 25.0f };
	const FVector3 lightDirection = { 0.577f, -0.577f, 0.577f };
	constexpr float lightIntensity = 7.0f / static_cast<float>(E_PI);
	const Elite::RGBColor lightColor = { 1.0f, 1.0f, 1.0f };

	Elite::RGBColor finalColor = diffuse * (lightColor * lightIntensity * std::max(Elite::Dot(-normal, lightDirection), 0.0f)) + ambient;

	const float dotProduct = Elite::Dot(lightDirection - (2 * Elite::Dot(normal, lightDirection) * normal), viewDirection);

	if (dotProduct > 0)
		finalColor += specular * std::powf(dotProduct, phongExponent * shininess);

	finalColor.MaxToOne();

	return finalColor;
}

Elite::IVector4 Elite::Renderer::GetBoundingBox(const std::vector<Vertex_Input>& vertices) const
{
	// Bounding Box
		// x = minX
		// y = maxX
		// z = minY
		// w = maxY
	Elite::IVector4 boundingBox{ INT_MAX, 0, INT_MAX, 0 };

	for (const Vertex_Input& vertex : vertices)
	{
		boundingBox.x = std::min(boundingBox.x, static_cast<int>(vertex.Position.x));
		boundingBox.y = std::max(boundingBox.y, static_cast<int>(vertex.Position.x) + 1);
		boundingBox.z = std::min(boundingBox.z, static_cast<int>(vertex.Position.y));
		boundingBox.w = std::max(boundingBox.w, static_cast<int>(vertex.Position.y) + 1);
	}
	// Clamp 
	boundingBox.x = std::max(boundingBox.x, 0);
	boundingBox.y = std::min(boundingBox.y, static_cast<int>(m_Width));
	boundingBox.z = std::max(boundingBox.z, 0);
	boundingBox.w = std::min(boundingBox.w, static_cast<int>(m_Height));

	return boundingBox;
}

bool Elite::Renderer::IsPointInTriangle(float w0, float w1, float w2) const
{
	switch (m_CullMode)
	{
	case CullMode::backface:
		return (w0 <= 0 && w1 <= 0 && w2 <= 0);

	case CullMode::frontface:
		return (w0 >= 0 && w1 >= 0 && w2 >= 0);

	case CullMode::none:
		return ((w0 <= 0 && w1 <= 0 && w2 <= 0) || (w0 >= 0 && w1 >= 0 && w2 >= 0));

	default:
		return false;
	}
}

void Elite::Renderer::ResetDepthBuffer() const
{
	for (uint32_t i = 0; i < m_Height * m_Width; i++)
		m_DepthBuffer[i] = FLT_MAX;
}
