#include "Material.h"




Material::Material(SimpleVertexShader * vertexShader, SimplePixelShader * pixelShader, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView *normal, ID3D11ShaderResourceView* roughness, ID3D11ShaderResourceView* metal, ID3D11SamplerState * sampler)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->albedoSRV = albedo;
	this->normalSRV = normal;
	this->roughnessSRV = roughness;
	this->metalSRV = metal;
	this->sampler = sampler;
}

Material::~Material()
{
}

SimpleVertexShader * Material::GetVertexShader()
{
	return vertexShader;
}

SimplePixelShader * Material::GetPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView * Material::GetAlbedoSRV()
{
	return albedoSRV;
}

ID3D11SamplerState * Material::GetSampler()
{
	return sampler;
}
ID3D11ShaderResourceView * Material::GetNormalSRV()
{
	return normalSRV;
}

ID3D11ShaderResourceView * Material::GetRoughnessSRV()
{
	return roughnessSRV;
}

ID3D11ShaderResourceView * Material::GetMetalSRV()
{
	return metalSRV;
}
