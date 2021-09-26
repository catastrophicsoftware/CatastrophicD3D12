cbuffer CBMaterial : register(b0)
{
	float4 color;
}


float4 main() : SV_TARGET
{
	float4 output = color;
	output.a = 1.0f;
	return output;
}