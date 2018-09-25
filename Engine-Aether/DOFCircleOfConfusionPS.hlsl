cbuffer Data : register(b0)
{
	//float nearBlurryPlaneZ;
	//float nearSharpPlaneZ;
	//float farSharpPlaneZ;
	//float farBlurryPlaneZ;
	float focusPlaneZ;
	float scale; //what does it do
	//float nearScale;
	//float4 pixelColor; //float farScale;
	//float2 screenSpaceUV; //FragCoord
	//float3 clipInfo; //clips off pixels on the side //why?
	//float2 writeScaleBias;
	//int2 trimBandThickness;
	float4x4 viewMatrixInv;
	float4x4 projMatrixInv;

}

struct VertexToPixel
{
	float4 position					: SV_POSITION;
	float2 uv						: TEXCOORD0;
};

Texture2D Pixels					: register(t0);
Texture2D DepthBuffer				: register(t1);
SamplerState Sampler				: register(s0);

float3 WorldPosFromDepth(float depth, float2 uv) 
{
	float z = depth * 2.0 - 1.0;

	// Get clip space
	float4 clipSpacePosition = float4(uv * 2.0 - 1.0, z, 1.0);

	// Clip space -> View space
	float4 viewSpacePosition = mul(projMatrixInv, clipSpacePosition);

	// Perspective division
	viewSpacePosition /= viewSpacePosition.w;

	// View space -> World space
	float4 worldSpacePosition = mul(viewMatrixInv, viewSpacePosition);

	return worldSpacePosition.xyz;
}

float reconstructCSZ(float d)
{
	return (2/ (1 * d + 0.1));
}

float4 main(VertexToPixel input) : SV_TARGET
{
	//input.uv = screenSpaceUV + trimBandThickness;	//what is trimbandthickness
	//color = texelFetch(ColorBuffer, uv, 0).rgb; //we are not packing color?

	float z = DepthBuffer.Sample(Sampler, input.uv).r;
	//z = WorldPosFromDepth(DepthBuffer.Sample(Sampler, input.uv).r, input.uv); //texelFetch = load?

	float radius = (reconstructCSZ(z) - focusPlaneZ) * scale;

	//radius = radius *writeScaleBias.x + writeScaleBias.y;

	return float4(radius, 0, 0, 0);
}