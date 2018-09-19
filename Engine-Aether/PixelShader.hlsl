
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
SamplerState basicSampler	: register(s0);


struct VertexToPixel
{
	float4 position			: SV_POSITION;
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;

};

float4 main(VertexToPixel input) : SV_TARGET
{
	//return float4(0,0,1,1);
	input.normal = normalize(input.normal);
	//for the texture calculation
	
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


	//return light2.DiffuseColor * dirNdotL2 + light.AmbientColor;

	return float4(lightOne, 1);
}
