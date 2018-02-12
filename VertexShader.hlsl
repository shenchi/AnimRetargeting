cbuffer frameConstants : register (b0)
{
	matrix matProj;
	matrix matView;
	matrix matModel;
	float3 cameraPos;
}

cbuffer boneBuffer : register(b1)
{
	matrix bones[128];
}

struct VSInput
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float2		texcoord	: TEXCOORD0;
	uint4		boneId		: BLENDINDICES;
	float4		weights		: BLENDWEIGHT;
};

struct VSOutput
{
	float4		position	: SV_POSITION;
	float3		normal		: NORMAL;
	float2		texcoord	: TEXCOORD0;
};

VSOutput main( VSInput input )
{
	VSOutput output;

	matrix matBone =
		bones[input.boneId.x] * input.weights.x +
		bones[input.boneId.y] * input.weights.y +
		bones[input.boneId.z] * input.weights.z +
		bones[input.boneId.w] * input.weights.w;

	matBone = mul(matBone, matModel);

	matrix matMVP = mul(mul(matBone, matView), matProj);

	output.position = mul(input.position, matMVP);
	output.normal = normalize(mul(input.normal, (float3x3)matBone));
	output.texcoord = input.texcoord;

	return output;
}