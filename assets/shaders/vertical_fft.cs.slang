import fft_common;

Texture2D<float4> input;
RWTexture2D<float4> output;

[[vk::push_constant]]
uint32_t flag;

[numthreads(fft::SIZE, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    // 1 - inverse, 0 - forward
    bool is_inverse = flag == 0 ? false : true;
    output[tid.yx] = fft::apply_fft(tid.x, float4(input[tid.yx]), is_inverse);
}
