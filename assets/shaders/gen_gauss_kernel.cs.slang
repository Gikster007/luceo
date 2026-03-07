RWTexture2D<float4> output;

static const uint N = 512;
static const float SIGMA = 10.0f; // tweak this to control blur radius

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // Center the Gaussian at pixel (0,0) with wrap-around.
    // This is the correct layout for FFT convolution — no fftshift needed.
    int2 pos = int2(tid.xy);
    if (pos.x > int(N / 2)) pos.x -= int(N);
    if (pos.y > int(N / 2)) pos.y -= int(N);

    const float dist2 = dot(float2(pos), float2(pos));
    float gauss = exp(-dist2 / (2.0f * SIGMA * SIGMA));

    // Normalize so the kernel sums to ~1 (analytical normalization)
    gauss /= (2.0f * 3.14159265f * SIGMA * SIGMA);

    output[tid.xy] = float4(gauss, gauss, gauss, 1.0);
}