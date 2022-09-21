#include "BasicShaderHeader.hlsli"

Output BasicVS( float4 pos : POSITION ,float2 uv:TEXCOORD)
{
	Output o;
	o.pos = mul(mvp,pos);
	o.uv = uv;
	return o;
}