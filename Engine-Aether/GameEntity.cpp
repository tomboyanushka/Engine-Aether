#include "GameEntity.h"


GameEntity::GameEntity(Mesh * mesh, Material* newMat)
{

	this->meshObject = mesh;
	this->materialObject = newMat;
	this->SetTranslation(XMFLOAT3(0.0, 0.0, 0.0));
	this->SetScale(XMFLOAT3(1.0, 1.0, 1.0));
	//rotation = XMFLOAT3(1.0, 1.0, 1.0);
	//this->Rotate(1, 1, 1);
	this->SetRotation(0, 0, 0);
	GetMatrix();

}

GameEntity::~GameEntity()
{
}

void GameEntity::SetTranslation(XMFLOAT3 setPos)
{
	position = setPos;
}

void GameEntity::SetScale(XMFLOAT3 setScale)
{
	scale = setScale;
}

void GameEntity::SetRotation(float x, float y, float z)
{
	rotation = XMFLOAT3(x,y,z);
}

void GameEntity::Move(float x, float y, float z)
{
	position.x += x;	
	position.y += y;	
	position.z += z; 
}

void GameEntity::Rotate(float x, float y, float z)
{
	rotation.x += x;	
	rotation.y += y;	
	rotation.z += z;
}


Mesh * GameEntity::GetMesh()
{
	return meshObject;
}

Material * GameEntity::GetMaterial()
{
	return materialObject;
}


XMFLOAT4X4 GameEntity::GetMatrix() //returning the world matrix 
{
	//coverting them to vectors
	XMVECTOR vPosition = XMLoadFloat3(&position);
	XMVECTOR vRotation = XMLoadFloat3(&rotation);
	XMVECTOR vScale = XMLoadFloat3(&scale);

	//coverting to matrices
	XMMATRIX mPosition = XMMatrixTranslationFromVector(vPosition);
	XMMATRIX mRotation = XMMatrixRotationRollPitchYawFromVector(vRotation);
	XMMATRIX mScale = XMMatrixScalingFromVector(vScale);

	
	XMMATRIX wake = XMMatrixIdentity();

	//calculte world matrix
	XMMATRIX world = mScale * mRotation * mPosition;

	//storing the world matrix

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));

	return worldMatrix;
}

void GameEntity::UpdateWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotX = XMMatrixRotationX(rotation.x);
	XMMATRIX rotY = XMMatrixRotationY(rotation.y);
	XMMATRIX rotZ = XMMatrixRotationZ(rotation.z);
	XMMATRIX sc = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX total = sc * rotZ * rotY * rotX * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(total));
}

void GameEntity::PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projMatrix, ID3D11SamplerState * objSampler)
{
	auto vertexShader = materialObject->GetVertexShader();
	auto pixelShader = materialObject->GetPixelShader();

	//first SRV, then sample it
	pixelShader->SetShaderResourceView("someTexture", materialObject->GetSRV());
	pixelShader->SetShaderResourceView("NormalTexture", materialObject->GetNormalSRV());
	pixelShader->SetSamplerState("basicSampler", objSampler);

	vertexShader->SetMatrix4x4("World", GetMatrix());
	vertexShader->SetMatrix4x4("View", viewMatrix);
	vertexShader->SetMatrix4x4("Proj", projMatrix);

	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();

	vertexShader->SetShader();
	pixelShader->SetShader();
}

