#include "ParticleIncludes.hlsli"
#include "SimplexNoise.hlsli"

cbuffer ExternalData : register(b0)
{
	float TotalTime;
	int EmitCount;
	int MaxParticles;
	int GridSize;
}

RWStructuredBuffer<Particle> ParticlePool	: register(u0);
ConsumeStructuredBuffer<uint> DeadList		: register(u1);

[numthreads(32, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{
	if (id.x >= (uint)EmitCount)
		return;

	uint emitIndex = DeadList.Consume();

	//replace grid with sphere
	float3 gridPos;
	uint gridIndex = emitIndex;
	gridPos.x = gridIndex % (GridSize + 1);
	gridIndex /= (GridSize + 1);
	gridPos.y = gridIndex % (GridSize + 1);
	gridIndex /= (GridSize + 1);
	gridPos.z = gridIndex;

	Particle emitParticle = ParticlePool.Load(emitIndex);

	//color age position size velocity alive 

	emitParticle.Color = float4(1,1,1,1);
	emitParticle.Age = 0.0f;
	emitParticle.Position = gridPos / 10 - float3(GridSize / 20.0f, GridSize / 20.0f, -GridSize / 10.0f);
	emitParticle.Size = 0.05f;
	emitParticle.Velocity = float3(0, 0, 0);
	emitParticle.Alive = 1.0f;

	ParticlePool[emitIndex] = emitParticle;
}