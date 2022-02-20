struct DirectionalLight
{
	float4 Direction; //Direction.w = light_enabled (0.0 or 1.0)
	float4 AmbientColor;
	float4 DiffuseColor;
	float4 Intensity; //x = ambient_intensity, y = diffuse_intensity
};

cbuffer CBMaterial : register(b0)
{
	float4 materialDiffuseColor;
}

cbuffer CBLightingData : register(b1)
{
	DirectionalLight directional_lights[8];
}

struct PixelInput
{
	float4 position : SV_Position;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

Texture2D    diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

float4 main(PixelInput input) : SV_TARGET
{
	float4 output = float4(0,0,0,0);

	input.normal = normalize(input.normal);

	float4 lightColor = float4(1, 1, 1, 1);
	float3 lightDirection = float3(0, 1, 0);

	float4 diffuselight = float4(1, 1, 1, 1) * dot(input.normal, lightDirection);
	output += float4(1, 1, 1, 1) * 0.25f; //ambient light

	output += diffuselight;

	float4 diffuseTexSample = diffuseTexture.Sample(samplerState, input.uv);
	output += diffuseTexSample;

	output.a = 1.0f;
	return output;
}