#include "Emitter.h"


Emitter::Emitter(unsigned int maxParticles, float emissionRate, float lifetime, ID3D11Device * device, ID3D11DeviceContext * context, SimpleComputeShader * deadListInitCS, SimpleComputeShader * emitCS, SimpleComputeShader * updateCS, SimpleComputeShader * copyDrawCountCS, SimpleVertexShader * particleDefaultVS, SimplePixelShader * particlePS, SimpleVertexShader * particleVS)
{
	this->maxParticles = maxParticles;
	this->emissionRate = emissionRate;
	this->lifetime = lifetime;
	this->context = context;
	this->emitCS = emitCS;
	this->updateCS = updateCS;
	this->copyDrawCountCS = copyDrawCountCS;
	this->particleDefaultVS = particleDefaultVS;
	this->particleVS = particleVS;
	this->particlePS = particlePS;

	emitTimeCounter = 0.0f;
	timeBetweenEmit = 1.0f;

	//scoping bc it's good practice
	//Particle index buffer
	{
		//Buffer
		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = sizeof(unsigned long) * maxParticles * 6;
		ibDesc.CPUAccessFlags = 0;
		ibDesc.MiscFlags = 0;
		ibDesc.StructureByteStride = 0;
		ibDesc.Usage = D3D11_USAGE_DEFAULT;

		//Data for Buffer
		unsigned long* indices = new unsigned long[maxParticles * 6];
		for (unsigned long i = 0; i < maxParticles; ++i)
		{
			unsigned long indexCounter = i * 6;
			indices[indexCounter + 0] = 0 + i * 4;	//why and from where
			indices[indexCounter + 1] = 1 + i * 4;	//0-1-2-1-3-2
			indices[indexCounter + 2] = 2 + i * 4;
			indices[indexCounter + 3] = 1 + i * 4;
			indices[indexCounter + 4] = 3 + i * 4;
			indices[indexCounter + 5] = 2 + i * 4;
		}

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = indices;
		device->CreateBuffer(&ibDesc, &data, &indexBuffer);

		delete[] indices;
	}

	//Particle Pool
	{
		//Buffer
		ID3D11Buffer* particlePoolBuffer;
		D3D11_BUFFER_DESC poolDesc = {};
		poolDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		poolDesc.ByteWidth = sizeof(Particle) * maxParticles;
		poolDesc.CPUAccessFlags = 0;
		poolDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		poolDesc.StructureByteStride = sizeof(Particle);
		poolDesc.Usage = D3D11_USAGE_DEFAULT;
		device->CreateBuffer(&poolDesc, 0, &particlePoolBuffer);

		//UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC poolUAVDesc = {};
		poolUAVDesc.Format = DXGI_FORMAT_UNKNOWN; 
		poolUAVDesc.Buffer.FirstElement = 0;
		poolUAVDesc.Buffer.Flags = 0;
		poolUAVDesc.Buffer.NumElements = maxParticles;
		poolUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		device->CreateUnorderedAccessView(particlePoolBuffer, &poolUAVDesc, &particlePoolUAV);

		//SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC poolSRVDesc = {};
		poolSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		poolSRVDesc.Buffer.FirstElement = 0;
		poolSRVDesc.Buffer.NumElements = maxParticles;
		// DO NOT SET THESE because it is unioned with above data and will just overwrite correct values with incorrect values
		//poolSRVDesc.Buffer.ElementOffset = 0;
		//poolSRVDesc.Buffer.ElementWidth = sizeof(Particle);
		poolSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		device->CreateShaderResourceView(particlePoolBuffer, &poolSRVDesc, &particlePoolSRV);

		//Release
		particlePoolBuffer->Release();
	}

	//Dead List 
	{}

	//Draw List
	{}
}

Emitter::~Emitter()
{
}

void Emitter::Update(float dt, float tt)
{
}

void Emitter::Draw(Camera * camera, float aspectRatio, float width, float height, bool additive)
{
}
