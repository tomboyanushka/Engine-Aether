cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	float blurAmount;
	float4x4 viewMatrixInv;
	float4x4 projMatrixInv;
	float focusPlaneZ;
	float zFar;
	float zNear;
}

struct VertexToPixel
{
	float4 position					: SV_POSITION;
	float2 uv						: TEXCOORD0;
};

Texture2D Pixels					: register(t0);
Texture2D DepthBuffer				: register(t1);
SamplerState Sampler				: register(s0);

float CalcLinearZ(float depth)
{
	float2 projectionConstants;
	projectionConstants.x = zFar / (zFar - zNear);
	projectionConstants.y = (-zFar * zNear) / (zFar - zNear);
	float linearZ = projectionConstants.y / (depth - projectionConstants.x);
	return linearZ;

}
//float3 WorldPosFromDepth(float depth, float2 uv) 
//{
//	float z = CalcLinearZ(depth) * 2.0 - 1.0;
//
//	// Get clip space
//	float4 clipSpacePosition = float4(uv * 2.0 - 1.0, z, 1.0);
//
//	// Clip space -> View space
//	float4 viewSpacePosition = mul(projMatrixInv, clipSpacePosition);
//
//	// Perspective division
//	viewSpacePosition /= viewSpacePosition.w;
//
//	// View space -> World space
//	float4 worldSpacePosition = mul(viewMatrixInv, viewSpacePosition);
//
//	return worldSpacePosition.xyz;
//}

float4 main(VertexToPixel input) : SV_TARGET
{
	
	float4 totalColor = float4(0, 0, 0, 0);
	uint numSamples = 0;

	int blurAmount = 3.0f;
	float depth = DepthBuffer.Sample(Sampler, input.uv).r;
	float linearZ = CalcLinearZ(depth);
	//float3 WorldZ = WorldPosFromDepth(DepthBuffer.Sample(Sampler, input.uv).r, input.uv);

	if (linearZ > focusPlaneZ + 3 || linearZ < focusPlaneZ - 1)
	{
		for (int x = -blurAmount; x <= blurAmount; x++)
		{
			for (int y = -blurAmount; y <= blurAmount; y++)
			{
				float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);
				totalColor += Pixels.Sample(Sampler, uv);

				numSamples++;
			}
		}

	}
	else
	{
		totalColor = Pixels.Sample(Sampler, input.uv);
		numSamples = 1;
	}
	
	return totalColor / numSamples;
}