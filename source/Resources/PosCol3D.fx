//----------------------
// Globals
//----------------------
float gPI					: PI;
float gLightIntensity		: LightIntensity;
float gShininess			: Shininess;

float3 gLightDirection 		: LightDirection;

float4x4 gWorldViewProj 	: WorldViewProjection;
float4x4 gWorldMatrix 		: WorldMatrix;
float4x4 gViewInverse 		: ViewInverseMatrix;

Texture2D gDiffuseMap 		: DiffuseMap;
Texture2D gNormalMap 		: NormalMap;
Texture2D gSpecularMap 		: SpecularMap;
Texture2D gGlossinessMap	: GlossinessMap;

SamplerState gSampler		: Sampler;
RasterizerState gRasterizer : Rasterizer;
BlendState gBlend			: Blend;

//----------------------
// Input/Output Structs
//----------------------
struct VS_INPUT
{
	float3 Position 		: POSITION;
	float2 UV 				: TEXCOORD;
	float3 Normal 			: NORMAL;
	float3 Tangent 			: TANGENT;
	float3 Color 			: COLOR;
};

struct VS_OUTPUT
{
	float4 Position 		: SV_POSITION;
	float2 UV 				: TEXCOORD;
	float3 Normal 			: NORMAL;
	float3 Tangent 			: TANGENT;
	float4 WorldPosition 	: COLOR;
};

//----------------------
// DepthStencilStates
//----------------------
DepthStencilState gDepthStencilState
{
    DepthWriteMask = all;
};

//----------------------
// Vertex Shader
//----------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.UV = input.UV;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	return output;
}

//----------------------
// Pixel Shader Functions
//----------------------
float4 SampleTexture(SamplerState mySampler, float2 UV)
{
	float4 textureSample = gDiffuseMap.Sample(mySampler, UV);
	return textureSample;
}

float4 SampleNormal(SamplerState mySampler, float2 UV)
{
	float4 normalSample = gNormalMap.Sample(mySampler, UV);
	return normalSample;
}

float SampleSpecular(SamplerState mySampler, float2 UV)
{
	float4 specularSample = gSpecularMap.Sample(mySampler, UV);
	return specularSample.r;
}

float SampleGlossiness(SamplerState mySampler, float2 UV)
{
	float4 glossinessSample = gGlossinessMap.Sample(mySampler, UV);
	return glossinessSample.r;
}

float4 CalcLambert(float observedArea, float specularSample, float4 textureSample, float4 lightColor)
{
    float4 lambert = (specularSample * textureSample) / gPI;
    return (lightColor * gLightIntensity * lambert * observedArea);
}

float4 CalcPhong(float observedArea, float specularSample, float glossinessSample, float3 viewDirection, float3 normal)
{
    float4 phong = float4(0.f, 0.f, 0.f, 1.f);
    if (observedArea >= 0)
    {
        float3 reflect = -gLightDirection - 2 * (observedArea * (normal));
        float angle = clamp(dot(reflect, viewDirection), 0.f, 1.f);
        phong = specularSample * pow(angle, int(glossinessSample * gShininess));
    }
    return phong;
}

//----------------------
// Pixel Shaders
//----------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
	
	float3 binormal = normalize(cross(input.Normal, input.Tangent));
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
	
    float4 sampledNormal = SampleNormal(gSampler, input.UV);
	sampledNormal.xyz = 2.f * sampledNormal.xyz - 1.f;
	
    float3 newNormal = normalize(mul(sampledNormal.xyz, tangentSpaceAxis));
	float observedArea = dot(newNormal, -gLightDirection);
	
    float4 lightColor = float4(1.f, 1.f, 1.f, 1.f);
    return saturate(CalcLambert(observedArea, SampleSpecular(gSampler, input.UV), SampleTexture(gSampler, input.UV), lightColor) + CalcPhong(observedArea, SampleSpecular(gSampler, input.UV), SampleGlossiness(gSampler, input.UV), viewDirection, newNormal));
}
//----------------------
// Technique
//----------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizer);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}