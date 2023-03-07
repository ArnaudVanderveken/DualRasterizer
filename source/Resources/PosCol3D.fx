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

float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float3 gLightColor = float3(1.0f, 1.0f, 1.0f);
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gAmbientColor = float3(0.025f, 0.025f, 0.025f);

float gPI = 3.1415926536f;

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
	BlendEnable[0] = false;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = one;
	DestBlendAlpha = one;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};
//---------------------------------------
// DepthStencilState
//---------------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = true;
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
// Pixel Shaders
//---------------------------------------

float4 ShadePixel(VS_OUTPUT input, SamplerState sampleState)
{
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

	// Normal Calculations
	float3 normalSample = gNormalMap.Sample(sampleState, input.UV).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, cross(input.Normal, input.Tangent), input.Normal);
	float3 trueNormal = normalize(mul(normalSample, tangentSpaceAxis));

	// Diffuse
	float3 diffuseSample = gDiffuseMap.Sample(sampleState, input.UV).xyz;
	float3 diffuseColor = gLightColor * (gLightIntensity / gPI) * diffuseSample * max(dot(trueNormal, -gLightDirection), 0.0f);

	// Specular
	float phongExponent = gGlossinessMap.Sample(sampleState, input.UV).r;
	float3 specularSample = gSpecularMap.Sample(sampleState, input.UV);
	float dotProduct = saturate(dot(reflect(-gLightDirection, trueNormal), viewDirection));
	float3 specularColor = specularSample * pow(dotProduct, phongExponent * gShininess);

	return float4(diffuseColor + specularColor + gAmbientColor, 1.0f);
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
