#ifndef __PARTICLE_INCLUDES
#define __PARTICLE_INCLUDES
struct Particle
{
	//color age position size velocity alive 
	float4 Color;
	float Age;
	float3 Position;
	float Size;
	float3 Velocity;
	float Alive;
	float3 padding;
};

struct ParticleDraw
{
	uint Index;
	float DistanceSq;
};

float3 CalcGridPos(uint index, int gridSize)
{
	gridSize += 1;

	float3 gridPos;
	gridPos.x = index % gridSize;
	index /= gridSize;
	gridPos.y = index % gridSize;

	index /= gridSize;
	gridPos.z = index;

	return gridPos;
}

#endif