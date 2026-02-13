struct VertexOutput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

static const float2 fs_triangle_pos[3] = float2[](
    float2(-1.0f, -1.0f),
    float2( 3.0f, -1.0f),
    float2(-1.0f,  3.0f)
);

[shader("vertex")]
VertexOutput main(uint32_t vid: SV_VertexID) {
    VertexOutput output;
    output.pos = float4(fs_triangle_pos[vid], 0.0f, 1.0f);
    output.uv = output.pos.xy * 0.5f + 0.5f;
    return output;
}