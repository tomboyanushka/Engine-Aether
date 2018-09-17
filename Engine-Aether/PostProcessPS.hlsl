cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	float blurAmount;
}

struct VertexToPixel
{
	float4 position					: SV_POSITION;
	float2 uv						: TEXCOORD0;
};

Texture2D Pixels					: register(t0);
SamplerState Sampler				: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 totalColor = float4(0, 0, 0, 0);
	uint numSamples = 0;

	int blurAmount = 3.0f;

	for (int x = -blurAmount; x <= blurAmount; x++)
	{
		for (int y = -blurAmount; y <= blurAmount; y++)
		{
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);
			totalColor += Pixels.Sample(Sampler, uv);

			numSamples++;
		}
	}

	return totalColor / numSamples;
}