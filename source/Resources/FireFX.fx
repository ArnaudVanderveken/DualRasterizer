

//---------------------------------------
// Global Variables
//---------------------------------------
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gViewInverseMatrix : ViewInverseMatrix;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

//---------------------------------------
// Input/Output Structs
//---------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 ViewDirection : VIEWDIR;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//---------------------------------------
// Function Declarations
//---------------------------------------
float4 ShadePixel(VS_OUTPUT input, SamplerState sampleState);

//---------------------------------------
// SamplerStructs
//---------------------------------------

// POINT FILTER
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};
// LINEAR FILTER
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
// ANISOTROPIC FILTER
SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

//---------------------------------------
// BlendStates
//---------------------------------------
BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};
//---------------------------------------
// DepthStencilState
//---------------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0f;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};
//---------------------------------------
// RasterizerStates
//---------------------------------------
RasterizerState gRasterizerStateBackCulling
{
	CullMode = back;
	FrontCounterClockwise = true;
};
RasterizerState gRasterizerStateFrontCulling
{
	CullMode = front;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateNoCulling
{
	CullMode = none;
	FrontCounterClockwise = true;
};
//---------------------------------------
// Vertex Shader
//---------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.UV = input.UV;
	output.Normal = normalize(mul(input.Normal, (float3x3) gWorldMatrix));
	output.Tangent = normalize(mul(input.Tangent, (float3x3) gWorldMatrix));

	return output;
}

//---------------------------------------
// Pixel Shaders
//---------------------------------------

float4 ShadePixel(VS_OUTPUT input, SamplerState sampleState)
{
	return gDiffuseMap.Sample(sampleState, input.UV);
}

// POINT FILTER
float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixel(input, samPoint);
}
// LINEAR FILTER
float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixel(input, samLinear);
}
// ANISOTROPIC PHONG FILTER
float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixel(input, samAnisotropic);
}


//---------------------------------------
// Techniques
//---------------------------------------

// Point FILTER
technique11 PointTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}
	pass P1
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	pass P2
	{
		SetRasterizerState(gRasterizerStateNoCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
}
// LINEAR FILTER
technique11 LinearTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	pass P1
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	pass P2
	{
		SetRasterizerState(gRasterizerStateNoCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
}
// ANISOTROPIC FILTER
technique11 AnisotropicTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
	pass P1
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	pass P2
	{
		SetRasterizerState(gRasterizerStateNoCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
}