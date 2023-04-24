#pragma once
#include "pch.h"
#include <vector>
#include "EMath.h"

#include "Effect.h"
#include "EffectPartialCoverage.h"

#include "structs.h"
#include "Triangle.h"

class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, const std::string& filePath, const Elite::FVector3 position, bool isTransparent = false);
	~Mesh() = default;

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	const Elite::FVector3& GetPosition() const { return m_Position; }

	void Render(ID3D11DeviceContext* pDeviceContext);

	void SetWorldViewMatrix(const float* pData);
	void SetWorldMatrix(const float* pData);
	void SetViewInverseMatrix(const float* pData);

	void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView);
	void SetNormalMap(ID3D11ShaderResourceView* pResourceView);
	void SetSpecularMap(ID3D11ShaderResourceView* pResourceView);
	void SetGlossinessMap(ID3D11ShaderResourceView* pResourceView);
	void SetTextureSamplingState(SampleMode renderTechnique);

	void SetCullMode(CullMode cullMode);

	const std::vector<uint32_t>& GetIndexBuffer() const;
	const std::vector<Vertex_Input>& GetVertexBuffer() const;
	Triangle* GetTriangle() const;
	void SetTemplateVertices(const std::vector<Vertex_Input>& localVertices);

private:
	Elite::FVector3 m_Position;

	unique_ptr<BaseEffect> m_pEffect;
	ComPtr<ID3D11InputLayout> m_pVertexLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	uint32_t m_AmountIndices;

	ComPtr<ID3DX11EffectMatrixVariable> m_pMatWorldViewProjVariable;
	ComPtr<ID3DX11EffectMatrixVariable> m_pMatWorldVariable;
	ComPtr<ID3DX11EffectMatrixVariable> m_pMatViewInverseVariable;
	ComPtr<ID3DX11EffectShaderResourceVariable> m_pDiffuseMapVariable;
	ComPtr<ID3DX11EffectShaderResourceVariable> m_pNormalMapVariable;
	ComPtr<ID3DX11EffectShaderResourceVariable> m_pSpecularMapVariable;
	ComPtr<ID3DX11EffectShaderResourceVariable> m_pGlossinessMapVariable;

	CullMode m_CullMode = CullMode::backface;

	unique_ptr<Triangle> m_pTriangle;
	std::vector<uint32_t> m_SWIndexBuffer;
	std::vector<Vertex_Input> m_SWVertexBuffer;
};

