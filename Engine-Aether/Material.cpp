#include "Material.h"




Material::Material(SimpleVertexShader * vertexShader, SimplePixelShader * pixelShader, ID3D11ShaderResourceView * shaderRV, ID3D11SamplerState * sampler)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
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

ID3D11ShaderResourceView * Material::GetSRV()
{
	return textureShader;
}

ID3D11SamplerState * Material::GetSampler()
{
	return sampler;
}
