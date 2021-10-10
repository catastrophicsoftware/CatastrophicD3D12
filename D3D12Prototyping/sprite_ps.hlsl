struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

Texture2D spriteTexture : register(t0);
SamplerState samplerState : register(s0);

float4 main(PixelInput input) : SV_TARGET
{
	float4 spriteTextureSample = spriteTexture.Sample(samplerState,input.texcoord);
	spriteTextureSample.a = 1.0f;

	return spriteTextureSample;
}