Texture2D<float4> input;
RWTexture2D<float4> output;

// Must match the values in generate_aperture.cs.slang
static const float N = 512.0;
static const uint NUM_BLADES = 6;
static const float RADIUS = 0.1;
static const float PI = 3.14159265;

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 val = input[tid.xy];

    // |complex|² = real² + imag²
    float i0 = val.x * val.x + val.y * val.y;
    float i1 = val.z * val.z + val.w * val.w;

    // Area of regular polygon in pixels:
    // A = (n/2) * R² * sin(2π/n), where R = RADIUS * N
    float R = RADIUS * N;
    float area = 0.5 * float(NUM_BLADES) * R * R * sin(2.0 * PI / float(NUM_BLADES));

    // Normalize: 1/N² for FFT scaling, 1/area for energy preservation
    float norm = 1.0 / (N * N * area);

    output[tid.xy] = float4(i0 * norm, 0.0, i1 * norm, 0.0);
}