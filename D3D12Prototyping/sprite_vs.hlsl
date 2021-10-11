struct VertexInput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

cbuffer CBPerFrame : register(b0)
{
	float4x4 cameraTransform;
}

cbuffer CBDynamic : register(b1)
{
	float4x4 spriteTransform;
}

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.texcoord = input.texcoord;

	output.position = mul(input.position, spriteTransform);
	output.position = mul(output.position, cameraTransform);

	return output;
}