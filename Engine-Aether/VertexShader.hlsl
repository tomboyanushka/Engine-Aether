cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;        // Normals
	float2 uv           : TEXCOORD;     //UVs
	float3 tangent		: TANGENT;

};


//must match pixel shader's input
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv           : TEXCOORD;
	float3 tangent		: TANGENT;
	float linearZ		: LINEARZ;
};

float2 GetProjectionConstants(float nearZ, float farZ)
{
	float2 projectionConstants;
	projectionConstants.x = farZ / (farZ - nearZ);
	projectionConstants.y = (-farZ * nearZ) / (farZ - nearZ);
	return projectionConstants;
}

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	float2 ProjectionConstants = GetProjectionConstants(0.1f, 100.f);

	matrix worldViewProj = mul(mul(world, view), projection);

	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	//normal to world space
	output.normal = mul(input.normal, (float3x3)world);

	// Also convert the tangent from LOCAL to WORLD space
	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	output.uv = input.uv;

	float depth = output.position.z / output.position.w;
	
	output.linearZ = ProjectionConstants.y / (depth - ProjectionConstants.x);

	return output;
}