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
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	output.normal = normalize(mul(input.normal, world)); //transform normal into world space
	output.uv = input.uv; //pass through texcoord

	return output;
}