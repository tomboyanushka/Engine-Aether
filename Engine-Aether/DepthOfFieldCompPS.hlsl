//cbuffer Data : register(b0)
//{
//
//}
//
struct VertexToPixel
{
	float4 position					: SV_POSITION;
	float2 uv						: TEXCOORD0;
};
//
//Texture2D Pixels					: register(t0);
//Texture2D BlurTexture				: register(t1);
//Texture2D Radius					: register(t2);
//SamplerState Sampler				: register(s0);


float4 main(VertexToPixel input) : SV_TARGET
{
//	float normalisedRadius = BlurTexture.Sample(Sampler, uv);

	return float4(1,0,0,0);

}