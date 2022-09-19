#include "BasicShaderHeader.hlsli"

Output BasicVS( float4 pos : POSITION ,float2 uv:TEXCOORD)
{
	Output o;
	o.pos = pos;
	o.uv = uv;
	return o;
}