cbuffer CBMaterial : register(b0)
{
	float4 materialDiffuseColor;
}

struct PixelInput
{
	float4 position : SV_Position;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

float4 main(PixelInput input) : SV_TARGET
{
	float4 output = float4(0,0,0,0);

	float4 lightColor = float4(1, 1, 1, 1);
	float3 lightDirection = float3(0, 1, 0);

	float4 diffuselight = float4(1, 1, 1, 1) * dot(input.normal, lightDirection);
	output += float4(1, 1, 1, 1) * 0.1f; //ambient light

	output += diffuselight;
	output.a = 1.0f;
	return output;
}