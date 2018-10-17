#include "Material.h"




Material::Material(SimpleVertexShader * vertexShader, SimplePixelShader * pixelShader, ID3D11ShaderResourceView *SRV, ID3D11ShaderResourceView *NormalSRV, ID3D11SamplerState * sampler)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->materialSRV = SRV;
	this->normalSRV = NormalSRV;
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
	return materialSRV;
}

ID3D11SamplerState * Material::GetSampler()
{
	return sampler;
}
ID3D11ShaderResourceView * Material::GetNormalSRV()
{
	return normalSRV;
}
