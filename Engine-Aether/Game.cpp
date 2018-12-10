#include "Game.h"
#include "DirectXMath.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "ObjLoader.h"

using namespace DirectX;

Vertex MapObjlToVertex(objl::Vertex vertex)
{
	auto pos = XMFLOAT3(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
	auto normal = XMFLOAT3(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
	auto uv = XMFLOAT2(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y);
	return { pos, normal, uv };
}

std::vector<Vertex> MapObjlToVertex(std::vector<objl::Vertex> vertices)
{
	std::vector<Vertex> verts;
	for (auto v : vertices)
	{
		verts.push_back(MapObjlToVertex(v));
	}
	return verts;
}

Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,								
		"Engine Aether ",		
		1280,									
		720,									
		true)									
{

	vertexBuffer = 0;
	indexBuffer = 0;
	vertexShader = 0;
	pixelShader = 0;
	camera = 0;

#if defined(DEBUG) || defined(_DEBUG)

	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully. Feel free to printf() here ");

#endif
}

//cleaning everything, even memory leaks
Game::~Game()
{
	if(vertexShader) delete vertexShader;
	if (pixelShader) delete pixelShader;

	if (quadPS) delete quadPS;
	if (quadVS) delete quadVS;
	if (blurPS) delete blurPS;
	if (cocPS) delete cocPS;
	if (dofPS) delete dofPS;
	if (particlePS) delete particlePS;
	if (particleEmitCS) delete particleEmitCS;
	if (particleUpdateCS) delete particleUpdateCS;
	if (particleDeadListInitCS) delete particleDeadListInitCS;
	if (particleCopyDrawCountCS) delete particleCopyDrawCountCS;
	if (particleDefaultVS) delete particleDefaultVS;
	if (particleVS) delete particleVS;
	if (skyPS) delete skyPS;
	if (skyVS) delete skyVS;

	if (noiseCS) delete noiseCS;

	if (ppRTV) ppRTV->Release();
	if (ppSRV) ppSRV->Release();
	if (blurRTV) blurRTV->Release();
	if (blurSRV) blurSRV->Release();
	if (cocRTV) cocRTV->Release();
	if (cocSRV) cocSRV->Release();
	if (dofRTV) dofRTV->Release();
	if (dofSRV) dofSRV->Release();
	if (depthBufferSRV) depthBufferSRV->Release();

	earthSRV->Release();
	earthNormalSRV->Release();
	scratchedA->Release();
	scratchedM->Release();
	scratchedN->Release();
	scratchedR->Release();

	cobbleA->Release();
	cobbleM->Release();
	cobbleN->Release();
	cobbleR->Release();

	lavaA->Release();
	lavaN->Release();
	lavaR->Release();
	lavaM->Release();

	waterA->Release();
	waterN->Release();
	waterM->Release();
	waterR->Release();

	woodA->Release();
	woodN->Release();
	woodM->Release();
	woodR->Release();

	skySRV->Release();
	skyIrradiance->Release();
	skyPrefilter->Release();
	brdfLookUpTexture->Release();
	sampler->Release();

	skyDepthState->Release();
	skyRasterState->Release();

	if(computeTextureSRV) computeTextureSRV->Release();
	if(computeTextureUAV) computeTextureUAV->Release();

	delete earthMaterial;
	delete marsMaterial;
	delete saturnMaterial;
	delete sphereMaterial;
	delete cubeMaterial;

	delete earthMesh;
	delete marsMesh;
	delete sphereMesh;
	delete cubeMesh;
	delete skyMesh;
	delete camera;

	delete particleEmitter;
	for (auto e : entities)
	{
		delete e;
	}

	

}

void Game::Init()
{
	//Ambient, Diffuse, Direction
	light1 = { XMFLOAT4(+0.1f, +0.1f, +0.1f, 1.0f), XMFLOAT4(+0.2f, +0.2f, +0.2f, +1.0f), XMFLOAT3(+1.0f, +0.0f, 0.8f) };
	light2 = { XMFLOAT4(+0.1f, +0.1f, +0.1f, 1.0f), XMFLOAT4(+1.0f, +0.0f, +0.0f, +1.0f), XMFLOAT3(+1.0f, +0.0f, 0.0f) };
	//color position range intensity
	light3 = { XMFLOAT4(+0.1f, +0.8f, +0.0f, 1.0f), XMFLOAT3(-1.0f, +0.0f, 2.0f), float (5), float (1) };
	//color position intensity direction range spotfalloff
	light4 = { XMFLOAT4(+0.7f, +0.4f, +0.1f, 1.0f), XMFLOAT3(5.0f, +2.0f,2.0f), float(5), XMFLOAT3(-1.0f, -1.0f, 0.0f), float(50), float (10) };
	LoadShaders();
	CreateMatrices();

	InitTextures();
	InitializeComputeShader();

	//Load skybox texture from DDS file
	//CreateDDSTextureFromFile(device, L"../../Assets/Textures/orbitalSkybox.dds", 0, &skySRV);
	CreateDDSTextureFromFile(device, L"../../Assets/Textures/Skybox/envEnvHDR.dds", 0, &skySRV);
	
	//IBL Baker Textures
	CreateDDSTextureFromFile(device, L"../../Assets/Textures/Skybox/envDiffuseHDR.dds", 0, &skyIrradiance);
	CreateDDSTextureFromFile(device, L"../../Assets/Textures/Skybox/envSpecularHDR.dds", 0, &skyPrefilter);
	CreateDDSTextureFromFile(device, L"../../Assets/Textures/Skybox/envBrdf.dds", 0, &brdfLookUpTexture);



	CreateMesh();
	//creating a sampler state for the textures
	D3D11_SAMPLER_DESC sd = {};
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	//sd.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR; //tri-linear
	
	sd.Filter = D3D11_FILTER_ANISOTROPIC;
	sd.MaxAnisotropy = 16;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sd, &sampler);

	//Create states for sky rendering
	D3D11_RASTERIZER_DESC rs = {};
	rs.CullMode = D3D11_CULL_FRONT;
	rs.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&rs, &skyRasterState);

	D3D11_DEPTH_STENCIL_DESC ds = {};
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);



	//Texture2D
	//RTV
	//SRV

	//post processing 
	ID3D11Texture2D* postProcTexture;
	ID3D11Texture2D* blurTexture;
	ID3D11Texture2D* cocTexture;
	ID3D11Texture2D* dofTexture;

	//setting up render texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&textureDesc, 0, &postProcTexture);
	device->CreateTexture2D(&textureDesc, 0, &blurTexture);
	device->CreateTexture2D(&textureDesc, 0, &cocTexture);
	device->CreateTexture2D(&textureDesc, 0, &dofTexture);


	//set up render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(postProcTexture, &rtvDesc, &ppRTV);
	device->CreateRenderTargetView(blurTexture, &rtvDesc, &blurRTV);
	device->CreateRenderTargetView(cocTexture, &rtvDesc, &cocRTV);
	device->CreateRenderTargetView(dofTexture, &rtvDesc, &dofRTV);

	//setting up shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(postProcTexture, &srvDesc, &ppSRV);
	device->CreateShaderResourceView(blurTexture, &srvDesc, &blurSRV);
	device->CreateShaderResourceView(cocTexture, &srvDesc, &cocSRV);
	device->CreateShaderResourceView(dofTexture, &srvDesc, &dofSRV);

	

	postProcTexture->Release();
	blurTexture->Release();
	cocTexture->Release();
	dofTexture->Release();

	//particle setup
	particleEmitter = new Emitter(
		1000000,
		1000000.0f, // Particles per second
		1000.0f, // Particle lifetime
		device,
		context,
		particleDeadListInitCS,
		particleEmitCS,
		particleUpdateCS,
		particleCopyDrawCountCS,
		particleDefaultVS,
		particlePS,
		particleVS);

	//what kind of shape do you want to draw?
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


}



//Load shaders from compile shader object (.cso) 
//and send data to individual variables on GPU
void Game::LoadShaders()
{	
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"SkyPS.cso");

	skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"SkyVS.cso");

	quadVS = new SimpleVertexShader(device, context);
	quadVS->LoadShaderFile(L"QuadVS.cso");

	quadPS = new SimplePixelShader(device, context);
	quadPS->LoadShaderFile(L"QuadPS.cso");

	blurPS = new SimplePixelShader(device, context);
	blurPS->LoadShaderFile(L"BlurPS.cso");

	cocPS = new SimplePixelShader(device, context);
	cocPS->LoadShaderFile(L"CircleOfConfusionPS.cso");

	dofPS = new SimplePixelShader(device, context);
	dofPS->LoadShaderFile(L"DepthOfFieldCompPS.cso");

	noiseCS = new SimpleComputeShader(device, context);
	if (!noiseCS->LoadShaderFile(L"Debug/ComputeShader.cso"))		//checking both for .exe and .cpp
		noiseCS->LoadShaderFile(L"ComputeShader.cso");

	particleEmitCS = new SimpleComputeShader(device, context);
	if (!particleEmitCS->LoadShaderFile(L"Debug/ParticleEmitCS.cso"))
		particleEmitCS->LoadShaderFile(L"ParticleEmitCS.cso");

	particleUpdateCS = new SimpleComputeShader(device, context);
	if (!particleUpdateCS->LoadShaderFile(L"Debug/ParticleUpdateCS.cso"))
		particleUpdateCS->LoadShaderFile(L"ParticleUpdateCS.cso");

	particleDeadListInitCS = new SimpleComputeShader(device, context);
	if (!particleDeadListInitCS->LoadShaderFile(L"Debug/ParticleDeadListInitCS.cso"))
		particleDeadListInitCS->LoadShaderFile(L"ParticleDeadListInitCS.cso");

	particleCopyDrawCountCS = new SimpleComputeShader(device, context);
	if (!particleCopyDrawCountCS->LoadShaderFile(L"Debug/ParticleCopyDrawCountCS.cso"))
		particleCopyDrawCountCS->LoadShaderFile(L"ParticleCopyDrawCountCS.cso");

	particleDefaultVS = new SimpleVertexShader(device, context);
	if (!particleDefaultVS->LoadShaderFile(L"Debug/ParticleDefaultVS.cso"))
		particleDefaultVS->LoadShaderFile(L"ParticleDefaultVS.cso");

	particleVS = new SimpleVertexShader(device, context);
	if (!particleVS->LoadShaderFile(L"Debug/ParticleVS.cso"))
		particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	if (!particlePS->LoadShaderFile(L"Debug/ParticlePS.cso"))
		particlePS->LoadShaderFile(L"ParticlePS.cso");
}


//initialized matrices necessary for geometry
void Game::CreateMatrices()
{
	//setting up the world matrix
	//transpose happens Because HLSL expects a different matrix than the DirectX Math Library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W));

	// view matrix
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,
		dir,
		up);

	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));

	//Projection Matrix
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,			//fov(field of view angle)
		(float)width / height,
		0.1f,
		100.0f);

	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));

	camera = new Camera(0, 0, -5);
	camera->UpdateProjectionMatrix((float)width / height);

}

void Game::CreateMesh()
{
	objl::Loader loader; //credits: https://github.com/Bly7/OBJ-Loader
	loader.LoadFile("../../Assets/Models/sphere.obj");
	auto verts = MapObjlToVertex(loader.LoadedVertices);
	auto indices = loader.LoadedMeshes[0].Indices;
	earthMesh = new Mesh();
	earthMesh->CreateBasicGeometry(verts.data(), (UINT)verts.size(), indices.data(), (UINT)indices.size(), device);

	loader.LoadFile("../Assets/Models/Mars.obj");
	auto marsverts = MapObjlToVertex(loader.LoadedVertices);
	auto marsindices = loader.LoadedMeshes[0].Indices;
	marsMesh = new Mesh();
	marsMesh->CreateBasicGeometry(marsverts.data(), (UINT)marsverts.size(), marsindices.data(), (UINT)marsindices.size(), device);


	loader.LoadFile("../../Assets/Models/sphere.obj");
	auto sphereverts = MapObjlToVertex(loader.LoadedVertices);
	auto sphereindices = loader.LoadedMeshes[0].Indices;
	sphereMesh = new Mesh();
	sphereMesh->CreateBasicGeometry(sphereverts.data(), (UINT)sphereverts.size(), sphereindices.data(), (UINT)sphereindices.size(), device);
	
	cubeMesh = new Mesh("../../Assets/Models/cube.obj", device);

	skyMesh = new Mesh("../../Assets/Models/cube.obj", device);
	

	//materials
	earthMaterial = new Material(vertexShader, pixelShader, computeTextureSRV, earthNormalSRV, earthSRV, 0, sampler);
	marsMaterial = new Material(vertexShader, pixelShader, lavaA, lavaN, lavaR, lavaM, sampler);
	saturnMaterial = new Material(vertexShader, pixelShader, waterA, waterN, waterR, waterM, sampler);
	sphereMaterial = new Material(vertexShader, pixelShader, scratchedA, scratchedN, scratchedR, scratchedM, sampler);
	cubeMaterial = new Material(vertexShader, pixelShader, woodA, woodN, woodR, woodM, sampler);

	

	//entities
	entities.push_back(new GameEntity(sphereMesh, earthMaterial));
	entities.push_back(new GameEntity(sphereMesh, marsMaterial));
	entities.push_back(new GameEntity(sphereMesh, saturnMaterial));
	entities.push_back(new GameEntity(sphereMesh, sphereMaterial));
	entities.push_back(new GameEntity(cubeMesh, cubeMaterial));

}

void Game::DrawSky()
{
	
	ID3D11Buffer* skyVB = skyMesh->GetVertexBuffer();
	ID3D11Buffer* skyIB = skyMesh->GetIndexBuffer();

	//set buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	//set up sky shaders
	//no need for world matrix
	skyVS->SetMatrix4x4("view", camera->GetViewMatrix());
	skyVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	skyVS->CopyAllBufferData();
	skyVS->SetShader();

	skyPS->SetShaderResourceView("SkyTexture", skySRV);
	skyPS->SetSamplerState("BasicSampler", sampler);
	skyPS->SetShader();

	//set up render states necessary for the sky
	context->RSSetState(skyRasterState);
	context->OMSetDepthStencilState(skyDepthState, 0);
	context->DrawIndexed(skyMesh->GetIndexCount(), 0, 0);

	// When done rendering, reset any and all states for the next frame
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

}

void Game::DrawBlur()
{
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &blurRTV, 0);

	//post process shaders
	quadVS->SetShader();
	blurPS->SetShader();

	blurPS->SetShaderResourceView("Pixels", ppSRV);
	blurPS->SetShaderResourceView("DepthBuffer", depthBufferSRV);
	blurPS->SetSamplerState("Sampler", sampler);
	blurPS->SetFloat("blurAmount", 5);
	blurPS->SetFloat("pixelWidth", 1.0f / width);
	blurPS->SetFloat("pixelHeight", 1.0f / height);
	blurPS->SetFloat("focusPlaneZ", focusZ);
	blurPS->SetFloat("zFar", 100.0f);
	blurPS->SetFloat("zNear", 0.1f);
	blurPS->CopyAllBufferData();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);
	blurPS->SetShaderResourceView("Pixels", 0);
	blurPS->SetShaderResourceView("DepthBuffer", 0);

}

void Game::DrawCircleofConfusion()
{
	//setting cocRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &cocRTV, 0);

	quadVS->SetShader();
	cocPS->SetShader();

	cocPS->SetShaderResourceView("Pixels", ppSRV);
	cocPS->SetSamplerState("Sampler", sampler);
	cocPS->SetShaderResourceView("DepthBuffer", depthBufferSRV);
	cocPS->SetFloat("focusPlaneZ", focusZ);
	cocPS->SetFloat("scale", 0.15f);
	cocPS->SetFloat("zFar", 100.0f);
	cocPS->SetFloat("zNear", 0.1f);
	cocPS->CopyAllBufferData();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);
	cocPS->SetShaderResourceView("Pixels", 0);
	cocPS->SetShaderResourceView("DepthBuffer", 0);
}

void Game::DrawDepthofField()
{
	//setting dofRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &dofRTV, 0);

	quadVS->SetShader();
	dofPS->SetShader();

	dofPS->SetShaderResourceView("Pixels", ppSRV);
	dofPS->SetSamplerState("Sampler", sampler);
	dofPS->SetShaderResourceView("BlurTexture", blurSRV);
	dofPS->SetShaderResourceView("Radius", cocSRV);

	dofPS->CopyAllBufferData();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);
	dofPS->SetShaderResourceView("Pixels", 0);
	dofPS->SetShaderResourceView("BlurTexture", 0);
	dofPS->SetShaderResourceView("Radius", 0);
}

void Game::DrawSimplex(float deltaTime, float totalTime)
{
	// Launch ("dispatch") compute shader
	noiseCS->SetInt("iterations", 8);
	noiseCS->SetFloat("persistence", 0.5f);
	noiseCS->SetFloat("scale", 0.01f);
	noiseCS->SetFloat("offset", totalTime);
	noiseCS->SetUnorderedAccessView("outputTexture", computeTextureUAV);
	noiseCS->SetShader();
	noiseCS->CopyAllBufferData();
	noiseCS->DispatchByThreads(noiseTextureSize, noiseTextureSize, 1);

	// Unbind the texture so we can use it later in draw
	noiseCS->SetUnorderedAccessView("outputTexture", 0);
}

void Game::InitializeComputeShader()
{
	noiseTextureSize = 256;
	ID3D11Texture2D* noiseTexture;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = noiseTextureSize;
	texDesc.Height = noiseTextureSize;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&texDesc, 0, &noiseTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(noiseTexture, &srvDesc, &computeTextureSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texDesc.Format;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(noiseTexture, &uavDesc, &computeTextureUAV);

	noiseTexture->Release();
}

void Game::InitTextures()
{
	//loading textures and normal maps
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Earth_Diffuse.jpg", 0, &earthSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Earth_Normal.jpg", 0, &earthNormalSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_albedo.png", 0, &scratchedA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_normals.png", 0, &scratchedN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_roughness.png", 0, &scratchedR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_metal.png", 0, &scratchedM);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_albedo.png", 0, &cobbleA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_normals.png", 0, &cobbleN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_roughness.png", 0, &cobbleR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_metal.png", 0, &cobbleM);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/lava/diffuse.jpg", 0, &lavaA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/lava/normal.jpg", 0, &lavaN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/lava/roughness.jpg", 0, &lavaR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/lava/metal.jpg", 0, &lavaM);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/water/wateralbedo.jpg", 0, &waterA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/water/waternormal.jpg", 0, &waterN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/water/waterroughness.png", 0, &waterR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/water/watermetal.jpg", 0, &waterM);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/floor/diffuse.png", 0, &woodA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/floor/normal.png", 0, &woodN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/floor/roughness.png", 0, &woodR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/floor/metal.png", 0, &woodM);
}

void Game::DrawEntity(GameEntity * gameEntityObject)
{

	vertexShader->SetMatrix4x4("world", gameEntityObject->GetMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	pixelShader->SetData("light1", &light1, sizeof(DirectionalLight));
	pixelShader->SetData("light2", &light2, sizeof(DirectionalLight));
	pixelShader->SetData("light3", &light3, sizeof(PointLight));
	pixelShader->SetData("light4", &light4, sizeof(SpotLight));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	gameEntityObject->PrepareMaterial(viewMatrix, projectionMatrix, sampler, skyIrradiance, skyPrefilter, brdfLookUpTexture);
	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	DrawMesh(gameEntityObject->GetMesh());

}


void Game::OnResize()
{
	DXCore::OnResize();

	//update projection matrix since window size has changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,
		(float)width / height,
		0.1f,
		100.0f);

	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));

	if (camera)
		camera->UpdateProjectionMatrix((float)width / height);
}

void Game::Update(float deltaTime, float totalTime)
{
	//quit if escape is hit
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	particleEmitter->Update(deltaTime, totalTime);

	camera->Update(deltaTime);

	DrawSimplex(deltaTime, totalTime);

	entities[0]->SetScale(XMFLOAT3(1, 1, 1));
	entities[0]->SetRotation(XM_PI, totalTime * 0.25f, 0.0);
	entities[0]->SetTranslation(XMFLOAT3(2, 0, 2));

	entities[1]->SetTranslation(XMFLOAT3(1.5f, 0, 5));

	//entities[2]->SetTranslation(XMFLOAT3(3, 0, 7));

	entities[2]->SetTranslation(XMFLOAT3(-1.5f, 0, 5));

	entities[3]->SetRotation(XM_PI, totalTime * 0.25f, 0.0);
	entities[3]->SetTranslation(XMFLOAT3(-2, 0, 2));

	//entities[5]->SetTranslation(XMFLOAT3(0, 0, 2));

	if (GetAsyncKeyState(VK_TAB))
	{
		isdofEnabled = true;
	}
	else
	{
		isdofEnabled = false;
	}

	// Handle particle update
	//static bool updateParticles = true;


	//if (updateParticles)
	//{
	
	//}

	
}


void Game::Draw(float deltaTime, float TotalTime)
{

	//bg color
	const float color[4] = { 0.1f, 0.1f, 0.1f, 0.0f };

	//do this before drawing ANYTHING
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(ppRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->OMSetRenderTargets(1, &ppRTV, depthStencilView);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	
	for (auto e : entities)
	{
		DrawEntity(e);
	}

	DrawSky();

	DrawBlur();

	DrawCircleofConfusion();

	DrawDepthofField();

	//setting backbufferRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	quadVS->SetShader();
	quadPS->SetShader();
	ID3D11ShaderResourceView* result = nullptr;
	if (isdofEnabled)
	{
		result = dofSRV;
	}
	else
	{
		result = ppSRV;
	}
	quadPS->SetShaderResourceView("Pixels", result);
	quadPS->SetSamplerState("Sampler", sampler);
	quadPS->CopyAllBufferData();

	context->Draw(3, 0);

	// Draw particles
	particleEmitter->Draw(camera, (float)width / height, (float)width, (float)height, true);


	swapChain->Present(0, 0);
	quadPS->SetShaderResourceView("Pixels", 0);

}

#pragma region Mouse Input

void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;

	SetCapture(hWnd);
}

void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
	}

	prevMousePos.x = x;
	prevMousePos.y = y;
}

void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	//add custom code for scrolling
}

void Game::DrawMesh(Mesh* mesh)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	auto vertexBuffer = mesh->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
}


#pragma endregion


