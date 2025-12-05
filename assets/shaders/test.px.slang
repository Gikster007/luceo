struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
};

[shader("fragment")]
float4 main() : SV_Target0 {
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}