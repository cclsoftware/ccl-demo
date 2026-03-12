#include <metal_stdlib>
using namespace metal;

struct PixelShaderInput
{
	float4 pos [[position]];
};

fragment float4 function (PixelShaderInput input [[stage_in]])
{
	// Draw the entire triangle yellow.
	return float4(1.0f, 1.0f, 0.0f, 1.0f);
}

