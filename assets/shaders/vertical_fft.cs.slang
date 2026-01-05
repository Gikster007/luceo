import fft_common;

Texture2D<float4> input;
RWTexture2D<float4> output;

[numthreads(fft::SIZE, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    output[tid.yx] = fft::apply_fft(tid.x, input[tid.yx]);
}
