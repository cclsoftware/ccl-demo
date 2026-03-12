struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
};

PixelShaderInput main (VertexShaderInput input)
{
    PixelShaderInput vertexShaderOutput;
    vertexShaderOutput.pos = float4 (input.pos, 1.0f);

    return vertexShaderOutput;
}
