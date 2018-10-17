#pragma once
#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, ID3D11ShaderResourceView* SRV, ID3D11ShaderResourceView* NormalSRV, ID3D11SamplerState* sampler);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();

	ID3D11ShaderResourceView* GetSRV();
	ID3D11ShaderResourceView* GetNormalSRV();
	ID3D11SamplerState* GetSampler();

private:

	SimpleVertexShader * vertexShader;
	SimplePixelShader* pixelShader;

	//ID3D11ShaderResourceView* textureShader;


	ID3D11ShaderResourceView* materialSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11SamplerState* sampler;



};

