#include "BasicShaderHeader.hlsli"

float4 PMDPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float brightness = dot(-light,input.normal);

	//float4 col = float4(tex.Sample(smp,input.uv));
	return float4(brightness,brightness,brightness,1);
}