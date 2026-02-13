module fft_common;

public struct Data
{
    public uint flag;
};

public namespace fft
{

public static const uint SIZE = 512;

static const uint LOG_SIZE = firstbithigh(SIZE);//log(SIZE) / log(2); // result of Log base 2 of SPECTRUM_TEX_SIZE

groupshared float4 fft_group_buffer[2][SIZE];

void ButterflyValues(uint step, uint index, out uint2 indices, out float2 twiddle, bool is_inverse)
{
    const float twoPi = 6.28318530718;
    uint b = SIZE >> (step + 1);
    uint w = b * (index / b);
    uint i = (w + index) % SIZE;
    sincos(-twoPi / SIZE * w, twiddle.y, twiddle.x);

    // This is what makes it the inverse FFT
    twiddle.y = is_inverse ? -twiddle.y : twiddle.y;
    indices = uint2(i, i + b);
}

float2 ComplexMult(float2 a, float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

public float4 apply_fft(uint threadIndex, float4 input, bool is_inverse)
{
    fft_group_buffer[0][threadIndex] = input;
    GroupMemoryBarrierWithGroupSync();

    uint flag = 0;

    /*[unroll]*/
    for (uint step = 0; step < LOG_SIZE; ++step)
    {
        uint2 inputsIndices;
        float2 twiddle;
        ButterflyValues(step, threadIndex, inputsIndices, twiddle, is_inverse);

        float4 v = fft_group_buffer[flag][inputsIndices.y];
        fft_group_buffer[1 - flag][threadIndex] =
            fft_group_buffer[flag][inputsIndices.x] +
            float4(ComplexMult(twiddle, v.xy), ComplexMult(twiddle, v.zw));

        flag ^= 1;
        GroupMemoryBarrierWithGroupSync();
    }

    const float scale = is_inverse ? (1.0f / float(SIZE)) : 1.0f;
    return fft_group_buffer[flag][threadIndex] * scale;
}

};
