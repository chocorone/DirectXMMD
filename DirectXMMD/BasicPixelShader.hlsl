float4 BasicPS(float4 pos:SV_POSITION) : SV_TARGET
{
	float x = pos.x * 0.0008f;
	float y = pos.y * 0.0018f;
	//float4 col = float4(x,0.0f, 1.0f-x, 1.0f);
	//float4 col = float4(0,1.0f-y, 0, 1.0f);
	//float4 col = float4(x, 1.0f - y, (1.0f - x), 1.0f);
	float4 col = float4(x*y, 1.0f - y, (1.0f - x)*y, 1.0f);
	return col;
}