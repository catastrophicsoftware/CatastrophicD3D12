cbuffer CBPerFrame : register(b0)
{
	matrix view;
	matrix projection;
}

cbuffer CBDynamic : register(b1)
{
	matrix world;
}


float4 main(float4 pos : SV_Position) : SV_POSITION
{
	pos = mul(pos,world);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	return pos;
}