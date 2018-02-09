struct VSInput
{
	float4		position	: POSITION;
	float4		normal		: NORMAL;
	float2		texcoord	: TEXCOORD0;
};

struct VSOutput
{
	float4		position	: SV_POSITION;
	float4		normal		: NORMAL;
	float2		texcoord	: TEXCOORD0;
};

VSOutput main( VSInput input )
{
	VSOutput output;
	output.position = input.position;
	output.normal = input.normal;
	output.texcoord = input.texcoord;

	return output;
}