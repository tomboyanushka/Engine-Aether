
struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float padding;

};

cbuffer externalData : register(b0)
{
	DirectionalLight light1;
	DirectionalLight light2;
};

//resources for the texture
Texture2D someTexture		: register(t0);
Texture2D NormalTexture		: register(t1);
SamplerState basicSampler	: register(s0);


struct VertexToPixel
{
	float4 position			: SV_POSITION;
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
	float3 tangent			: TANGENT;
	float linearZ			: LINEARZ;

};

// 10 - 1
//10.5

// 10.5/100 = 0.105

float PackFloat(float linearZ, float farZ)//float, farZ
{
	return linearZ / farZ;
}

float3 calculateNormalFromMap(float2 uv, float3 normal, float3 tangent)
{
	float3 normalFromTexture = NormalTexture.Sample(basicSampler, uv).xyz;
	float3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - N * dot(tangent, N));
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	return normalize(mul(unpackedNormal, TBN));
}

float4 main(VertexToPixel input) : SV_TARGET
{
	//return float4(0,0,1,1);
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	input.normal = calculateNormalFromMap(input.uv, input.normal, input.tangent);
	//for the normal calculation
	//Calculate the normalized direction to the light
	//Negate the light’s direction, normalize that and store in a float3 variable
	//for light 1
	float3 dir = -normalize(light1.Direction);
	float dirNdotL = dot(input.normal, dir);
	dirNdotL = saturate(dirNdotL); //clamps it between 0 and 1

	//final calculation
	//return light.DiffuseColor * dirNdotL + light.AmbientColor;// Dir light calc

	//for light 2
	float3 dir2 = -normalize(light2.Direction);
	float dirNdotL2 = dot(input.normal, dir2);
	dirNdotL2 = saturate(dirNdotL2);

	float4 surfaceColor = someTexture.Sample(basicSampler, input.uv);

	float3 lightOne = surfaceColor.rgb * (light1.DiffuseColor * dirNdotL + light1.AmbientColor);
	float3 lightTwo = surfaceColor.rgb * (light2.DiffuseColor * dirNdotL2 + light2.AmbientColor);

	float packedValue = PackFloat(input.linearZ, 100.0f);
	//return light2.DiffuseColor * dirNdotL2 + light.AmbientColor;
	float3 r = pow(lightOne, 1.0 / 2.2);
	return float4(r, packedValue);
}
