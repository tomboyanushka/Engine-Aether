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
		hInstance,								//handle
		"DirectX Game (Good Lord Save Me)",		//text for Title Bar
		1280,									//width
		720,									//height
		true)									//show extra stats
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
	delete vertexShader;
	delete pixelShader;

	delete ppPS;
	delete ppVS;
	delete blurPS;
	delete CoCPS;
	delete CoCVS;
	delete DoFPS;
	delete skyPS;
	delete skyVS;

	ppRTV->Release();
	ppSRV->Release();
	blurRTV->Release();
	blurSRV->Release();
	CoCRTV->Release();
	CoCSRV->Release();
	DoFRTV->Release();
	DoFSRV->Release();
	depthBufferSRV->Release();

	slateSRV->Release();
	slateNormalSRV->Release();
	earthSRV->Release();
	earthNormalSRV->Release();
	marsSRV->Release();
	marsNormalSRV->Release();
	neptuneSRV->Release();
	neptuneNormalSRV->Release();
	saturnSRV->Release();
	saturnNormalSRV->Release();
	scratchedA->Release();
	scratchedM->Release();
	scratchedN->Release();
	scratchedR->Release();
	cobbleA->Release();
	cobbleM->Release();
	cobbleN->Release();
	cobbleR->Release();
	skySRV->Release();
	sampler->Release();

	skyDepthState->Release();
	skyRasterState->Release();

	delete slateMaterial;
	delete earthMaterial;
	delete marsMaterial;
	delete neptuneMaterial;
	delete saturnMaterial;
	delete sphereMaterial;
	delete cubeMaterial;

	delete earthMesh;
	delete marsMesh;
	delete neptuneMesh;
	delete saturnMesh;
	delete sphereMesh;
	delete cubeMesh;
	delete skyMesh;
	delete camera;

	for (auto e : entities)
	{
		delete e;
	}

	

}

void Game::Init()
{
	//Ambient, Diffuse, Direction
	light1 = { XMFLOAT4(+0.1f, +0.1f, +0.1f, 1.0f), XMFLOAT4(+1.0f, +1.0f, +1.0f, +1.0f), XMFLOAT3(+1.0f, +0.0f, 0.8f) };
	light2 = { XMFLOAT4(+0.1f, +0.1f, +0.1f, 1.0f), XMFLOAT4(+1.0f, +0.0f, +0.0f, +1.0f), XMFLOAT3(+1.0f, +0.0f, 0.0f) };
	//color position range intensity
	light3 = { XMFLOAT4(+0.1f, +0.8f, +0.0f, 1.0f), XMFLOAT3(-1.0f, +0.0f, 2.0f), float (5), float (1) };
	//color position intensity direction range spotfalloff
	light4 = { XMFLOAT4(+1.0f, +0.1f, +0.1f, 1.0f), XMFLOAT3(8.0f, +0.0f, 3.0f), float(10), XMFLOAT3(-1.0f, 0.0f, 0.0f), float(10), float (25) };
	LoadShaders();
	CreateMatrices();

	//loading textures and normal maps
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/slate.tif", 0, &slateSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/slateNormal.tif", 0, &slateNormalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Earth_Diffuse.jpg", 0, &earthSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Earth_Normal.jpg", 0, &earthNormalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/mars.jpg", 0, &marsSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/marsNormal.jpg", 0, &marsNormalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/neptune.jpg", 0, &neptuneSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/neptuneNormal.jpg", 0, &neptuneNormalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Saturn.jpg", 0, &saturnSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/SaturnNormal.png", 0, &saturnNormalSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_albedo.png", 0, &scratchedA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_normals.png", 0, &scratchedN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_roughness.png", 0, &scratchedR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Scratched/scratched_metal.png", 0, &scratchedM);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_albedo.png", 0, &cobbleA);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_normals.png", 0, &cobbleN);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_roughness.png", 0, &cobbleR);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Cobble/cobblestone_metal.png", 0, &cobbleM);



	//Load skybox texture from DDS file
	//CreateDDSTextureFromFile(device, L"../../Assets/Textures/orbitalSkybox.dds", 0, &skySRV);
	CreateDDSTextureFromFile(device, L"../../Assets/Textures/Skybox/envEnvHDR.dds", 0, &skySRV);

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

	//Render the scene to texture.
	//Down sample the texture to to half its size or less.
	//Perform a horizontal blur on the down sampled texture.
	//Perform a vertical blur.
	//Up sample the texture back to the original screen size.
	//Render that texture to the screen.

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
	device->CreateRenderTargetView(cocTexture, &rtvDesc, &CoCRTV);
	device->CreateRenderTargetView(dofTexture, &rtvDesc, &DoFRTV);

	//setting up shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(postProcTexture, &srvDesc, &ppSRV);
	device->CreateShaderResourceView(blurTexture, &srvDesc, &blurSRV);
	device->CreateShaderResourceView(cocTexture, &srvDesc, &CoCSRV);
	device->CreateShaderResourceView(dofTexture, &srvDesc, &DoFSRV);

	postProcTexture->Release();
	blurTexture->Release();
	cocTexture->Release();
	dofTexture->Release();

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

	ppVS = new SimpleVertexShader(device, context);
	ppVS->LoadShaderFile(L"QuadVS.cso");

	ppPS = new SimplePixelShader(device, context);
	ppPS->LoadShaderFile(L"QuadPS.cso");

	blurPS = new SimplePixelShader(device, context);
	blurPS->LoadShaderFile(L"BlurPS.cso");

	CoCVS = new SimpleVertexShader(device, context);
	CoCVS->LoadShaderFile(L"CircleOfConfusionVS.cso");

	CoCPS = new SimplePixelShader(device, context);
	CoCPS->LoadShaderFile(L"CircleOfConfusionPS.cso");

	DoFPS = new SimplePixelShader(device, context);
	DoFPS->LoadShaderFile(L"DepthOfFieldCompPS.cso");
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

	loader.LoadFile("../Assets/Models/Neptune.obj");
	auto neptuneverts = MapObjlToVertex(loader.LoadedVertices);
	auto neptuneindices = loader.LoadedMeshes[0].Indices;
	neptuneMesh = new Mesh();
	neptuneMesh->CreateBasicGeometry(neptuneverts.data(), (UINT)neptuneverts.size(), neptuneindices.data(), (UINT)neptuneindices.size(), device);


	loader.LoadFile("../Assets/Models/Saturn.obj");
	auto saturnverts = MapObjlToVertex(loader.LoadedVertices);
	auto saturnindices = loader.LoadedMeshes[0].Indices;
	saturnMesh = new Mesh();
	saturnMesh->CreateBasicGeometry(saturnverts.data(), (UINT)saturnverts.size(), saturnindices.data(), (UINT)saturnindices.size(), device);

	loader.LoadFile("../../Assets/Models/sphere.obj");
	auto sphereverts = MapObjlToVertex(loader.LoadedVertices);
	auto sphereindices = loader.LoadedMeshes[0].Indices;
	sphereMesh = new Mesh();
	sphereMesh->CreateBasicGeometry(sphereverts.data(), (UINT)sphereverts.size(), sphereindices.data(), (UINT)sphereindices.size(), device);
	
	cubeMesh = new Mesh("../../Assets/Models/cube.obj", device);

	skyMesh = new Mesh("../../Assets/Models/cube.obj", device);
	

	//materials
	slateMaterial = new Material(vertexShader, pixelShader, slateSRV, slateNormalSRV, 0, 0, sampler);
	earthMaterial = new Material(vertexShader, pixelShader, earthSRV, earthNormalSRV, 0, 0, sampler);
	marsMaterial = new Material(vertexShader, pixelShader, marsSRV, marsNormalSRV, 0, 0, sampler);
	neptuneMaterial = new Material(vertexShader, pixelShader, neptuneSRV, neptuneNormalSRV, 0, 0, sampler);
	saturnMaterial = new Material(vertexShader, pixelShader, saturnSRV, saturnNormalSRV, 0, 0, sampler);
	sphereMaterial = new Material(vertexShader, pixelShader, scratchedA, scratchedN, scratchedR, scratchedM, sampler);
	cubeMaterial = new Material(vertexShader, pixelShader, cobbleA, cobbleN, cobbleR, cobbleM, sampler);

	

	//entities
	entities.push_back(new GameEntity(earthMesh, earthMaterial));
	entities.push_back(new GameEntity(marsMesh, marsMaterial));
	entities.push_back(new GameEntity(neptuneMesh, neptuneMaterial));
	entities.push_back(new GameEntity(saturnMesh, saturnMaterial));
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
	ppVS->SetShader();
	blurPS->SetShader();

	blurPS->SetShaderResourceView("Pixels", ppSRV);
	blurPS->SetShaderResourceView("DepthBuffer", depthBufferSRV);
	blurPS->SetSamplerState("Sampler", sampler);
	blurPS->SetFloat("blurAmount", 5);
	blurPS->SetFloat("pixelWidth", 1.0f / width);
	blurPS->SetFloat("pixelHeight", 1.0f / height);
	blurPS->SetMatrix4x4("viewMatrixInv", camera->GetViewMatrixInverse());
	blurPS->SetMatrix4x4("projMatrixInv", camera->GetProjectionMatrixInverse());
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
	//setting CoCRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &CoCRTV, 0);

	CoCVS->SetShader();
	CoCPS->SetShader();

	CoCPS->SetShaderResourceView("Pixels", ppSRV);
	CoCPS->SetSamplerState("Sampler", sampler);
	CoCPS->SetShaderResourceView("DepthBuffer", depthBufferSRV);
	CoCPS->SetFloat("focusPlaneZ", focusZ);
	CoCPS->SetFloat("scale", 1);
	CoCPS->SetFloat("zFar", 100.0f);
	CoCPS->SetFloat("zNear", 0.1f);
	CoCPS->CopyAllBufferData();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);
	CoCPS->SetShaderResourceView("Pixels", 0);
	CoCPS->SetShaderResourceView("DepthBuffer", 0);
}

void Game::DrawDepthofField()
{
	//setting DOFRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &DoFRTV, 0);

	ppVS->SetShader();
	DoFPS->SetShader();

	DoFPS->SetShaderResourceView("Pixels", ppSRV);
	DoFPS->SetSamplerState("Sampler", sampler);
	DoFPS->SetShaderResourceView("BlurTexture", blurSRV);
	DoFPS->SetShaderResourceView("Radius", CoCSRV);

	DoFPS->CopyAllBufferData();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);
	DoFPS->SetShaderResourceView("Pixels", 0);
	DoFPS->SetShaderResourceView("BlurTexture", 0);
	DoFPS->SetShaderResourceView("Radius", 0);
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

	gameEntityObject->PrepareMaterial(viewMatrix, projectionMatrix, sampler);
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

	camera->Update(deltaTime);

	entities[0]->SetScale(XMFLOAT3(1, 1, 1));
	entities[0]->SetRotation(XM_PI, totalTime * 0.25f, 0.0);
	entities[0]->SetTranslation(XMFLOAT3(2, 0, 3));

	entities[1]->SetTranslation(XMFLOAT3(1, 0, 5));

	entities[2]->SetTranslation(XMFLOAT3(3, 0, 7));

	entities[3]->SetTranslation(XMFLOAT3(-1.5f, 0, 6));

	entities[4]->SetRotation(XM_PI, totalTime * 0.25f, 0.0);
	entities[4]->SetTranslation(XMFLOAT3(-2, 0, 3));

	entities[5]->SetTranslation(XMFLOAT3(0, 0, 2));
	
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

	ppVS->SetShader();
	ppPS->SetShader();

	ppPS->SetShaderResourceView("Pixels", DoFSRV);
	ppPS->SetSamplerState("Sampler", sampler);
	ppPS->CopyAllBufferData();

	context->Draw(3, 0);

	swapChain->Present(0, 0);
	ppPS->SetShaderResourceView("Pixels", 0);

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


