cbuffer CBMaterial : register(b0)
{
	float4 color;
}


float4 main(float4 position : SV_Position, float4 normal : NORMAL, float2 uv : TEXCOORD) : SV_TARGET
{
	float4 output = float4(0,0,0,0);

	float4 lightColor = float4(1, 1, 1, 1);
	float3 lightDirection = float3(0, 1, 0);

	float4 diffuselight = float4(1, 1, 1, 1) * dot(normal, lightDirection);


	output += float4(1, 1, 1, 1) * 0.5f; //ambient light

	output += diffuselight;
	output.a = 1.0f;
	return output;
}