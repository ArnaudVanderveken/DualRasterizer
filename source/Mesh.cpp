#pragma once
#include "pch.h"
#include "Mesh.h" 
#include "EObjParser.h"

Mesh::Mesh(ID3D11Device* pDevice, const std::string& filePath, const Elite::FVector3 position, bool isTransparent)
	: m_pIndexBuffer{}
	, m_pVertexBuffer{}
	, m_pVertexLayout{}
	, m_pEffect{}
	, m_AmountIndices{}
	, m_pDiffuseMapVariable{}
	, m_pNormalMapVariable{}
	, m_pSpecularMapVariable{}
	, m_pGlossinessMapVariable{}
	, m_pMatViewInverseVariable{}
	, m_pMatWorldVariable{}
	, m_pMatWorldViewProjVariable{}
	, m_Position{ position }

	, m_pTriangle{ new Triangle{Elite::FPoint3(position)}}

{
	
	if (!Elite::ParseOBJ(filePath, position, m_SWVertexBuffer, m_SWIndexBuffer))
	{
		std::cout << "Parsing error with file " << filePath << std::endl;
		return;
	}

	if (!isTransparent) m_pEffect = new Effect(pDevice, L"Resources/PosCol3D.fx");
	else  m_pEffect = new EffectPartialCoverage(pDevice, L"Resources/FireFX.fx");

	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	// Position
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// UV
	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 16;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Normal
	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Tangent
	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// ViewDirection (unused in DirectX, only for Software compatibility)
	vertexDesc[4].SemanticName = "VIEWDIR";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 48;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create input layout
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout);
	if (FAILED(result)) 
	{
		std::cout << "Error when creating InputLayout!" << std::endl;
		return;
	}

	//Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_Input) * (uint32_t)m_SWVertexBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = m_SWVertexBuffer.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		std::cout << "Error when creating VertexBuffer!" << std::endl;
		return;
	}

	//Create index buffer
	m_AmountIndices = (uint32_t)m_SWIndexBuffer.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_SWIndexBuffer.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		std::cout << "Error when creating IndexBuffer!" << std::endl;
		return;
	}

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::cout << "m_pMatWorldViewProjVariable not valid.\n";

	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
		std::cout << "m_pMatWorldVariable not valid.\n";

	m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
	if (!m_pMatViewInverseVariable->IsValid())
		std::cout << "m_pMatViewInverseVariable not valid.\n";

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::cout << "m_pDiffuseMapVariable not valid.\n";

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::cout << "m_pNormalMapVariable not valid.\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::cout << "m_pSpecularMapVariable not valid.\n";

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
		std::cout << "m_pGlossinessMapVariable not valid.\n";
}

Mesh::~Mesh()
{
	// SoftwareRasterizer
	delete m_pTriangle;

	// DirectX
	m_pIndexBuffer->Release();
	m_pVertexBuffer->Release();
	m_pVertexLayout->Release();

	m_pMatWorldViewProjVariable->Release();
	m_pDiffuseMapVariable->Release();
	m_pNormalMapVariable->Release();
	m_pSpecularMapVariable->Release();
	m_pGlossinessMapVariable->Release();
	delete m_pEffect;
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	//Set vertex buffer
	UINT stride = sizeof(Vertex_Input);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Render a triangle
	m_pEffect->GetTechnique()->GetPassByIndex((int)m_CullMode)->Apply(0, pDeviceContext);
	pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
}

void Mesh::SetWorldViewMatrix(const float* pData)
{
	m_pMatWorldViewProjVariable->SetMatrix(pData);
}

void Mesh::SetWorldMatrix(const float* pData)
{
	m_pMatWorldVariable->SetMatrix(pData);
}

void Mesh::SetViewInverseMatrix(const float* pData)
{
	m_pMatViewInverseVariable->SetMatrix(pData);
}

void Mesh::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pDiffuseMapVariable->IsValid())
		m_pDiffuseMapVariable->SetResource(pResourceView);
	else std::cout << "Invalid DiffuseMap." << std::endl;
}

void Mesh::SetNormalMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pNormalMapVariable->IsValid())
		m_pNormalMapVariable->SetResource(pResourceView);
	else std::cout << "Invalid NormalMap." << std::endl;
}

void Mesh::SetSpecularMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pSpecularMapVariable->IsValid())
		m_pSpecularMapVariable->SetResource(pResourceView);
	else std::cout << "Invalid SpecularMap." << std::endl;
}

void Mesh::SetGlossinessMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pGlossinessMapVariable->IsValid())
		m_pGlossinessMapVariable->SetResource(pResourceView);
	else std::cout << "Invalid GlosinessMap." << std::endl;
}

void Mesh::SetTextureSamplingState(SampleMode renderTechnique)
{
	m_pEffect->SetTechnique(renderTechnique);
}

void Mesh::SetCullMode(CullMode cullMode)
{
	m_CullMode = cullMode;
}

const std::vector<uint32_t>& Mesh::GetIndexBuffer() const
{
	return m_SWIndexBuffer;
}

const std::vector<Vertex_Input>& Mesh::GetVertexBuffer() const
{
	return m_SWVertexBuffer;
}

Triangle* Mesh::GetTriangle() const
{
	return m_pTriangle;
}

void Mesh::SetTemplateVertices(const std::vector<Vertex_Input>& vertices)
{
	m_pTriangle->SetLocalVertices(vertices);
}