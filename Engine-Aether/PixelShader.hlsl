#include "Lighting.hlsli"
//struct DirectionalLight
//{
//	float4 AmbientColor;
//	float4 DiffuseColor;
//	float3 Direction;
//	float Intensity;
//
//};

cbuffer externalData : register(b0)
{
	DirectionalLight light1;
	DirectionalLight light2;
	PointLight light3;
	SpotLight light4;

	float3 CameraPosition;
};


//resources for the texture
Texture2D AlbedoTexture		: register(t0);
Texture2D NormalTexture		: register(t1);
Texture2D RoughnessTexture	: register(t2);
Texture2D MetalTexture		: register(t3);
SamplerState BasicSampler	: register(s0);


struct VertexToPixel
{
	float4 position			: SV_POSITION;
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
	float3 tangent			: TANGENT;
	float3 worldPos			: POSITION;
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
	float3 normalFromTexture = NormalTexture.Sample(BasicSampler, uv).xyz;
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
	float4 surfaceColor = AlbedoTexture.Sample(BasicSampler, input.uv);

	input.normal = calculateNormalFromMap(input.uv, input.normal, input.tangent);

	// Sample the roughness map
	float roughness = RoughnessTexture.Sample(BasicSampler, input.uv).r;
	//roughness = lerp(0.2f, roughness, 0.5f);

	// Sample the metal map
	float metalness = MetalTexture.Sample(BasicSampler, input.uv).r;
	//metal = lerp(0.0f, metal, 0.5f);

	// Specular color - Assuming albedo texture is actually holding specular color if metal == 1
	float3 specColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, 1);

	//float3 lightOne = surfaceColor.rgb * (light1.DiffuseColor * dirNdotL + light1.AmbientColor);
	//float3 lightTwo = surfaceColor.rgb * (light2.DiffuseColor * dirNdotL2 + light2.AmbientColor);
	// Total color for this pixel
	float3 totalColor = float3(0, 0, 0);
	
	float3 dirPBR = DirLightPBR(light1, input.normal, input.worldPos, CameraPosition, roughness, metalness, surfaceColor.rgb, specColor);
	float3 pointPBR = PointLightPBR(light3, input.normal, input.worldPos, CameraPosition, roughness, metalness, surfaceColor.rgb, specColor);
	float3 spotPBR = SpotLightPBR(light4, input.normal, input.worldPos, CameraPosition, roughness, metalness, surfaceColor.rgb, specColor);

	totalColor = dirPBR + pointPBR + spotPBR;

	float packedValue = PackFloat(input.linearZ, 100.0f);
	//return light2.DiffuseColor * dirNdotL2 + light.AmbientColor;
	float3 gammaCorrected = lerp(totalColor, pow(totalColor, 1.0 / 2.2), 0.4f);
	return float4(gammaCorrected, packedValue);
}
