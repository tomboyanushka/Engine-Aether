#include "Game.h"
#include "DirectXMath.h"
#include "WICTextureLoader.h"


using namespace DirectX;

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
	delete dofPS;
	delete dofVS;

	ppRTV->Release();
	ppSRV->Release();
	blurRTV->Release();
	blurSRV->Release();
	dofRTV->Release();
	dofSRV->Release();
	depthBufferSRV->Release();

	lavaSRV->Release();
	slateSRV->Release();
	sampler->Release();

	delete lavaMaterial;
	delete slateMaterial;

	delete sphereEntity;
	delete sphereMesh;
	delete cubeEntity;
	delete cubeMesh;
	delete camera;

}

void Game::Init()
{
	//Ambient, Diffuse, Direction
	light1 = { XMFLOAT4(+0.1f, +0.1f, +0.1f, 1.0f), XMFLOAT4(+1.0f, +0.0f, +0.6f, +1.0f), XMFLOAT3(+1.0f, +0.0f, 0.0f) };
	light2 = { XMFLOAT4(+0.1f, +0.1f, +0.1f, 1.0f), XMFLOAT4(+1.0f, +0.0f, +0.0f, +1.0f), XMFLOAT3(+1.0f, +0.0f, 0.0f) };
	LoadShaders();
	CreateMatrices();
	CreateMesh();

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/slate.tif", 0, &slateSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/rect.jpg", 0, &lavaSRV);

	
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
	device->CreateTexture2D(&textureDesc, 0, &dofTexture);


	//set up render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(postProcTexture, &rtvDesc, &ppRTV);
	device->CreateRenderTargetView(blurTexture, &rtvDesc, &blurRTV);
	device->CreateRenderTargetView(dofTexture, &rtvDesc, &dofRTV);

	//setting up shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(postProcTexture, &srvDesc, &ppSRV);
	device->CreateShaderResourceView(blurTexture, &srvDesc, &blurSRV);
	device->CreateShaderResourceView(dofTexture, &srvDesc, &dofSRV);

	postProcTexture->Release();
	blurTexture->Release();
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

	ppVS = new SimpleVertexShader(device, context);
	ppVS->LoadShaderFile(L"QuadVS.cso");

	ppPS = new SimplePixelShader(device, context);
	ppPS->LoadShaderFile(L"QuadPS.cso");

	blurPS = new SimplePixelShader(device, context);
	blurPS->LoadShaderFile(L"BlurPS.cso");

	dofPS = new SimplePixelShader(device, context);
	dofPS->LoadShaderFile(L"DOFCircleOfConfusionPS.cso");

	//DepthOfFieldVS = new SimpleVertexShader(device, context);
	//DepthOfFieldVS->LoadShaderFile(L"DOFCircleOfConfusion.cso");
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

	//materials
	lavaMaterial = new Material(vertexShader, pixelShader, lavaSRV, sampler);
	slateMaterial = new Material(vertexShader, pixelShader, slateSRV, sampler);

	//meshes
	sphereMesh = new Mesh("../../Assets/Models/sphere.obj", device);
	cubeMesh = new Mesh("../../Assets/Models/cube.obj", device);

	//entities
	sphereEntity = new GameEntity(sphereMesh, lavaMaterial);
	cubeEntity = new GameEntity(cubeMesh, slateMaterial);


}

void Game::DrawEntity(GameEntity * gameEntityObject, ID3D11ShaderResourceView* someSRV)
{

	vertexShader->SetMatrix4x4("world", gameEntityObject->GetMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	pixelShader->SetData("light1", &light1, sizeof(DirectionalLight));
	pixelShader->SetData("light2", &light2, sizeof(DirectionalLight));

	gameEntityObject->PrepareMaterial(viewMatrix, projectionMatrix, someSRV, sampler);
	//cubeEntity->PrepareMaterial(viewMatrix, projectionMatrix, slateSRV, sampler);

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

	sphereEntity->SetTranslation(XMFLOAT3(-1, 0, -1));
	sphereEntity->SetScale(XMFLOAT3(1.2, 1.2, 1.2));

	cubeEntity->SetTranslation(XMFLOAT3(1, 0, 0));
	cubeEntity->SetScale(XMFLOAT3(1, 1, 1));
	
	//sphereEntity->SetRotation(2);

}


void Game::Draw(float deltaTime, float TotalTime)
{

	//bg color
	const float color[4] = { 0.4f, 0.7f, 0.75f, 0.0f };

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

	DrawEntity(sphereEntity, slateSRV);
	DrawEntity(cubeEntity, lavaSRV);

	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &blurRTV, 0);

	//post process shaders
	ppVS->SetShader();
	blurPS->SetShader();

	blurPS->SetShaderResourceView("Pixels", ppSRV);
	blurPS->SetSamplerState("Sampler", sampler);
	blurPS->SetFloat("blurAmount", 5);
	blurPS->SetFloat("pixelWidth", 1.0f / width);
	blurPS->SetFloat("pixelHeight", 1.0f / height);
	blurPS->CopyAllBufferData();

	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);

	//setting dofRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &dofRTV, 0);

	ppVS->SetShader();
	dofPS->SetShader();

	dofPS->SetShaderResourceView("Pixels", ppSRV);
	dofPS->SetSamplerState("Sampler", sampler);
	dofPS->SetShaderResourceView("DepthBuffer", depthBufferSRV);
	dofPS->SetFloat("focusPlaneZ", 1);
	dofPS->SetFloat("scale", 1);
	dofPS->SetFloat("zFar", 100.0f);
	dofPS->SetFloat("zNear", 0.1f);
	//dofPS->SetFloat2("projectionConstants", XMFLOAT2(0.1f, 100.0f));
	dofPS->CopyAllBufferData();

	nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);

	//setting backbufferRTV
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	ppVS->SetShader();
	ppPS->SetShader();

	ppPS->SetShaderResourceView("Pixels", dofSRV);
	ppPS->SetSamplerState("Sampler", sampler);
	ppPS->CopyAllBufferData();

	context->Draw(3, 0);

	swapChain->Present(0, 0);

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


