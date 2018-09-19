cbuffer Data : register(b0)
{
	float focusPlaneZ;
	float scale; //what does it do
	//float nearBlurryPlaneZ;
	//float nearSharpPlaneZ;
	//float farSharpPlaneZ;
	//float farBlurryPlaneZ;
	float nearScale;
	float4 pixelColor; //float farScale;
	float4 screenSpace; //FragCoord
	float radius;
	float3 clipInfo; //clips off pixels on the side //why?
	float2 writeScaleBias;
	int2 trimBandThickness;
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
//Texture2D ColorBuffer				: register(t2);
SamplerState Sampler				: register(s0);

float3 WorldPosFromDepth(float depth, clipInfo) 
{
	float z = depth * 2.0 - 1.0;

	// Get clip space
	float4 clipSpacePosition = float4(uv * 2.0 - 1.0, z, 1);

	// Clip space -> View space
	float4 viewSpacePosition = projMatrixInv * clipSpacePosition;

	// Perspective division
	viewSpacePosition /= viewSpacePosition.w;

	// View space -> World space
	float4 worldSpacePosition = viewMatrixInv * viewSpacePosition;

	return worldSpacePosition.xyz;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	uv = screenSpace + trimBandThickness;	//what is trimbandthickness
	//color = texelFetch(ColorBuffer, uv, 0).rgb; //we are not packing color?

	float z;
	z = WorldPosFromDepth(Load(DepthBuffer, uv, 0).r, clipInfo); //texelFetch = load?

	radius = (z - focusPlaneZ) * scale;

	radius = radius * writeScaleBias.x + writeScaleBias.y;

	return(radius, 0, 0, 0);
}