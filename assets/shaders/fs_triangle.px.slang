Texture2D<float4> input_tex;
SamplerState sampler;

struct VertexOutput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

[shader("fragment")]
float4 main(VertexOutput input) : SV_Target0 {
    return float4(input_tex.Sample(sampler, input.uv).rgb, 1.0f);
}