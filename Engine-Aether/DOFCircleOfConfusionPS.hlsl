cbuffer Data : register(b0)
{

	float4x4 viewMatrixInv;
	float4x4 projMatrixInv;
	float focusPlaneZ;
	float scale;
	float zFar;
	float zNear;
	//float2 projectionConstants;
	

}

struct VertexToPixel
{
	float4 position					: SV_POSITION;
	float2 uv						: TEXCOORD0;
};

Texture2D Pixels					: register(t0);
Texture2D DepthBuffer				: register(t1);
SamplerState Sampler				: register(s0);

float UnpackFloat(float linearZ, float farZ)
{
	return linearZ * farZ;
}
//float CalcLinearZ(float depth)
//{
//	float2 projectionConstants;
//	projectionConstants.x = zFar / (zFar - zNear);
//	projectionConstants.y = (-zFar * zNear) / (zFar - zNear);
//	float linearZ = projectionConstants.y / (depth - projectionConstants.x);
//	return linearZ;
//
//}
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


//
//float reconstructCSZ(float d)
//{
//	return (2/ (1 * d + 0.1));
//}

float4 main(VertexToPixel input) : SV_TARGET
{	
	float linearZ = UnpackFloat(Pixels.Sample(Sampler, input.uv).a, 100);

 // 0.105 * 100 = 10.5
	float radius = (linearZ - focusPlaneZ) * scale;

	return float4(radius, 0, 0, 0);
}