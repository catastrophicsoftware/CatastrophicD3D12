cbuffer CBMaterial : register(b0)
{
	float4 color;
}


float4 main(float4 position : SV_Position,float4 normal : NORMAL, float2 uv : TEXCOORD) : SV_TARGET
{
	float4 output = color;

	float3 lightDirection = float3(1.0f, 1.0f, 1.0f);
	float NdL = dot(normal, lightDirection);

	float4 lightColor = float4(1, 1, 1, 1) * NdL * 5;
	output = saturate(lightColor);

	output.a = 1.0f;
	return output;
}