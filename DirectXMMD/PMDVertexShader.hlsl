#include "BasicShaderHeader.hlsli"

Output PMDVS(float4 pos : POSITION, float4 normal:NORMAL,float2 uv : TEXCOORD,min16uint2 boneno:BONE_NO,min16uint weight : WEIGHT)
{
	Output o;
	o.pos = mul(mvp, pos);
	o.uv = uv;
	normal.w = 0;
	o.normal = mul(world,normal);
	return o;
}