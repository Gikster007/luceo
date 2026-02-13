Texture2D<float4> input_rg;
Texture2D<float4> input_b;
RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 rg = input_rg[tid.xy];
    float4 b  = input_b[tid.xy];

    // After inverse FFT, the real parts hold the spatial values.
    float3 color = float3(rg.x, rg.z, b.x);

    output[tid.xy] = float4(color, 1.0f);
}