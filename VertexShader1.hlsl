cbuffer frameConstants : register (b0)
{
	matrix matProj;
	matrix matView;
	matrix matModel;
	float3 cameraPos;
}

struct VSInput
{
	float4		position	: POSITION;
	float4		color		: COLOR0;
};

struct VSOutput
{
	float4		position	: SV_POSITION;
	float4		color		: COLOR0;
};

VSOutput main(VSInput input)
{
	VSOutput output;

	matrix matMVP = mul(mul(matModel, matView), matProj);

	output.position = mul(input.position, matMVP);
	output.color = input.color;

	return output;
}