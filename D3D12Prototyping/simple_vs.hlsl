cbuffer CBPerFrame : register(b0)
{
	matrix view;
	matrix projection;
}

cbuffer CBDynamic : register(b1)
{
	matrix world;
}

struct VertexInput
{
	float4 position : SV_Position;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
};

VertexOutput main(VertexInput input) : SV_POSITION
{
	VertexOutput output;
	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	return output;
}