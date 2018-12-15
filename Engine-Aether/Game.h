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
#include "Emitter.h"
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
	void DrawBlur();
	void DrawCircleofConfusion();
	void DrawDepthofField();
	void DrawSimplex(float deltaTime, float totalTime);
	void InitializeComputeShader();
	void InitTextures();

	bool isdofEnabled = false;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	//skybox
	ID3D11ShaderResourceView* skySRV;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;
	ID3D11RasterizerState* skyRasterState;
	ID3D11DepthStencilState* skyDepthState;
	ID3D11DepthStencilState* depthWriteDisabled;

	//textures
	ID3D11SamplerState* sampler;
	ID3D11ShaderResourceView* earthSRV, *earthNormalSRV;
	ID3D11ShaderResourceView *scratchedA, *scratchedN, *scratchedR, *scratchedM;
	ID3D11ShaderResourceView *cobbleA, *cobbleN, *cobbleR, *cobbleM;
	ID3D11ShaderResourceView *groundA, *groundN, *groundR, *groundM;
	ID3D11ShaderResourceView *waterA, *waterN, *waterR, *waterM;
	ID3D11ShaderResourceView *woodA, *woodN, *woodR, *woodM;
	ID3D11ShaderResourceView *benchA, *benchN, *benchR, *benchM;

	//for IBL
	ID3D11ShaderResourceView* skyIrradiance;
	ID3D11ShaderResourceView* skyPrefilter;
	ID3D11ShaderResourceView* brdfLookUpTexture;


	//shaders
	SimpleVertexShader* quadVS;
	SimplePixelShader* quadPS;
	SimplePixelShader* blurPS;
	SimplePixelShader* cocPS;
	SimplePixelShader* dofPS;
	SimpleVertexShader* dofVS;

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Particle shaders
	SimpleVertexShader* particleDefaultVS;
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	SimpleComputeShader* particleEmitCS;
	SimpleComputeShader* particleUpdateCS;
	SimpleComputeShader* particleDeadListInitCS;
	SimpleComputeShader* particleCopyDrawCountCS;
	Emitter* particleEmitter;

	// Compute shader stuff
	int noiseTextureSize;
	SimpleComputeShader* noiseCS;
	ID3D11ShaderResourceView* computeTextureSRV;
	ID3D11UnorderedAccessView* computeTextureUAV;

	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;

	//meshes
	Mesh* earthMesh;
	Mesh* quadMesh;
	Mesh* sphereMesh;
	Mesh* cubeMesh;
	Mesh* skyMesh;
	Mesh* benchMesh;

	//entities
	std::vector<GameEntity*> entities;


	Material* earthMaterial;
	Material* marsMaterial;
	Material* saturnMaterial;
	Material* sphereMaterial;
	Material* cubeMaterial;
	Material* benchMaterial;

	Camera* camera;

	DirectionalLight light1;
	DirectionalLight light2;
	PointLight light3;
	SpotLight light4;

	void DrawEntity(GameEntity* gameEntityObject);

	POINT prevMousePos;

	bool prevP;
	//postprocess
	ID3D11RenderTargetView* ppRTV;
	ID3D11ShaderResourceView* ppSRV;

	ID3D11RenderTargetView* blurRTV;
	ID3D11ShaderResourceView* blurSRV;

	ID3D11RenderTargetView* cocRTV; //CircleOfConfusionRTV and SRV
	ID3D11ShaderResourceView* cocSRV;

	ID3D11RenderTargetView* dofRTV;
	ID3D11ShaderResourceView* dofSRV;


};

