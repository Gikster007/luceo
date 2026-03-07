import fft_common;

RWTexture2D<float4> image;
Texture2D<float4> kernel;

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 img = image[tid.xy];
    float4 ker = kernel[tid.xy];

    image[tid.xy] = float4(
        fft::ComplexMult(img.xy, ker.xy),
        fft::ComplexMult(img.zw, ker.zw)
    );
}