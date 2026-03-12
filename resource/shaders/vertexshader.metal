#include <metal_stdlib>
using namespace metal;

struct VertexData
{
	float3 pos [[attribute(0)]];
};

struct VertexShaderOutput
{
	float4 pos [[position]];
};

vertex VertexShaderOutput function (VertexData v [[stage_in]])
{
	VertexShaderOutput output;
	output.pos = vector_float4(0.0, 0.0, 0.0, 1.0);
	output.pos.xyz = v.pos;
	
	return output;
}
