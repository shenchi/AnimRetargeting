struct PSInput
{
	float4		position	: SV_POSITION;
	float3		normal		: NORMAL;
	float2		texcoord	: TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
	float NdotL = max(dot(input.normal, normalize(float3(1, 1, -1))), 0);

	float value = NdotL * 0.7 + 0.3;

	return float4(value, value, value, 1.0f);
}