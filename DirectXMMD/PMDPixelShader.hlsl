#include "BasicShaderHeader.hlsli"

float4 PMDPS(Output input) : SV_TARGET
{
	//float x = input.pos.x * 0.0008f;
	//float y = input.pos.y * 0.0018f;
	//float4 col = float4(x*y, 1.0f - y, (1.0f - x)*y, 1.0f);
	//float4 col = float4(input.uv.xy,1,1);
	//float4 col = float4(tex.Sample(smp,input.uv));
	return float4(0,0,0,1);
}