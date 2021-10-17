//----------------------
// Globals
//----------------------
float4x4 gWorldViewProj     : WorldViewProjection;
float4x4 gWorldMatrix       : WorldMatrix;

Texture2D gDiffuseMap       : DiffuseMap;

SamplerState gSampler       : Sampler;
RasterizerState gRasterizer : Rasterizer;
BlendState gBlend           : Blend;


//----------------------
// Input/Output Structs
//----------------------
struct VS_INPUT
{
    float3 Position         : POSITION;
    float2 UV               : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Color            : COLOR;
};

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 UV               : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float4 WorldPosition    : COLOR;
};

//----------------------
// DepthStencilStates
//----------------------
DepthStencilState gDepthStencilState
{
    DepthWriteMask = zero;
};

//----------------------
// Vertex Shader
//----------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.UV = input.UV;
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
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

//----------------------
// Pixel Shaders
//----------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return SampleTexture(gSampler, input.UV);
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
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}