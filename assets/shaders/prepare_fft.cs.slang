Texture2D<float4> input;
RWTexture2D<float4> output_rg;
RWTexture2D<float4> output_b;

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 color = input[tid.xy].rgb;

    // Pack R and G as two complex numbers: (R_real, R_imag, G_real, G_imag)
    output_rg[tid.xy] = float4(color.r, 0.0f, color.g, 0.0f);

    // Pack B as one complex number: (B_real, B_imag, 0, 0)
    output_b[tid.xy] = float4(color.b, 0.0f, 0.0f, 0.0f);
}