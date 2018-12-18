#include "ParticleIncludes.hlsli"
#include "SimplexNoise.hlsli"

cbuffer ExternalData : register(b0)
{
	float DT;
	float Lifetime;
	float TotalTime;
	int MaxParticles;
}

//Order should match EmitCS
RWStructuredBuffer<Particle> ParticlePool	: register(u0);
AppendStructuredBuffer<uint> DeadList		: register(u1);
RWStructuredBuffer<ParticleDraw> DrawList	: register(u2);

[numthreads(32, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{
	if (id.x >= (uint)MaxParticles) 
		return;

	Particle part = ParticlePool.Load(id.x);

	if (part.Alive == 0.0f)
		return;

	part.Age += DT;
	part.Alive = (float)(part.Age < Lifetime);
	part.Position += part.Velocity * DT;

	float2 noise = (frac(sin(dot(part.Position.xy, float2(12.9898, 78.233)*2.0)) * 43758.5453));

	//3D curl noise
	float3 curlPos = part.Position * 0.2f; // *noise.x;
	float3 curlVel = curlNoise3D(curlPos, 1.0f);
	part.Velocity = curlVel * 2;

	ParticlePool[id.x] = part;

	//newly dead
	if (part.Alive == 0.0f)
	{
		DeadList.Append(id.x);
	}
	else
	{
		uint drawIndex = DrawList.IncrementCounter();

		ParticleDraw drawData;
		drawData.Index = id.x;
		drawData.DistanceSq = 0.0f;
		//drawData.padding = float2(0, 0);
		DrawList[drawIndex] = drawData;
	}
}