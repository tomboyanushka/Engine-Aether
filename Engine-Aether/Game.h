#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXmath.h>
#include "Vertex.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include "Material.h "
#include <vector>
#include <stdlib.h>
#include <time.h>
using namespace DirectX;


class Game : public DXCore

{
public:
	Game(HINSTANCE hInstance);
	~Game();

	void Init();
	void OnResize();
	void Update(float deltaTime, float TotalTime);
	void Draw(float deltaTime, float TotalTime);

	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);


	void DrawMesh(Mesh* mesh);
	float focusZ = 5;


private:

	void LoadShaders();
	void CreateMatrices();
	void CreateMesh();
	void DrawSky();



	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	//skybox
	ID3D11ShaderResourceView* skySRV;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;
	ID3D11RasterizerState* skyRasterState;
	ID3D11DepthStencilState* skyDepthState;

	//textures
	ID3D11SamplerState* sampler;
	ID3D11ShaderResourceView* rectSRV;
	ID3D11ShaderResourceView* rectNormalSRV;
	ID3D11ShaderResourceView* earthSRV;
	ID3D11ShaderResourceView* earthNormalSRV;
	ID3D11ShaderResourceView* slateSRV;
	ID3D11ShaderResourceView* slateNormalSRV;

	//shaders
	SimpleVertexShader* ppVS;
	SimplePixelShader* ppPS;
	SimplePixelShader* blurPS;
	SimplePixelShader* CoCPS;
	SimpleVertexShader* CoCVS;
	SimplePixelShader* DoFPS;
	SimpleVertexShader* DofVS;

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;


	//meshes
	Mesh* sphereMesh;
	Mesh* earthMesh;
	Mesh* cubeMesh;
	std::vector<Mesh*> meshes;

	//entities
	GameEntity* sphereEntity;
	GameEntity* cubeEntity;
	GameEntity* earthEntity;
	std::vector<GameEntity*> entities;

	Material* lavaMaterial;
	Material* slateMaterial;
	Material* earthMaterial;
	Material* earthNormal;


	Camera* camera;

	DirectionalLight light1;
	DirectionalLight light2;

	void DrawEntity(GameEntity* gameEntityObject);

	POINT prevMousePos;


	//postprocess
	ID3D11RenderTargetView* ppRTV;
	ID3D11ShaderResourceView* ppSRV;

	ID3D11RenderTargetView* blurRTV;
	ID3D11ShaderResourceView* blurSRV;

	ID3D11RenderTargetView* CoCRTV; //CircleOfConfusionRTV and SRV
	ID3D11ShaderResourceView* CoCSRV;

	ID3D11RenderTargetView* DoFRTV;
	ID3D11ShaderResourceView* DoFSRV;


};

