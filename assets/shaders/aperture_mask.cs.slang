RWTexture2D<float4> output;

static const uint N = 512;
static const uint NUM_BLADES = 6;      // 5 = pentagon, 6 = hexagon, etc.
static const float RADIUS = 0.1;       // aperture radius in UV space (0-0.5)
static const float ROTATION = 0.0;     // rotation in radians
static const float PI = 3.14159265;

// SDF for a regular polygon centered at origin
// p = point, r = radius, n = number of sides
float sdf_polygon(float2 p, float r, uint n)
{
    // Angle per segment
    float seg_angle = 2.0 * PI / float(n);

    // Find which segment we're in and get angle within it
    float angle = atan2(p.y, p.x);
    // Snap to nearest segment center
    float a = floor((angle + seg_angle * 0.5) / seg_angle) * seg_angle;

    // Rotate point into segment-local space
    float2 d = float2(cos(a), sin(a));
    float2 n2 = float2(-sin(a), cos(a));

    // Distance to the edge of the polygon
    float2 local = float2(dot(p, d), abs(dot(p, n2)));
    float edge_dist = r * cos(PI / float(n));

    return local.x - edge_dist;
}

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // Center UV coordinates: (0,0) is center of image
    float2 uv = (float2(tid.xy) + 0.5) / float(N) - 0.5;

    // Apply rotation
    float cs = cos(ROTATION);
    float sn = sin(ROTATION);
    float2 rotated = float2(
        uv.x * cs - uv.y * sn,
        uv.x * sn + uv.y * cs
    );

    // Evaluate SDF — negative = inside
    float d = sdf_polygon(rotated, RADIUS, NUM_BLADES);

    // Hard edge: 1 inside, 0 outside
    float mask = d < 0.0 ? 1.0 : 0.0;

    output[tid.xy] = float4(mask, mask, mask, 1.0);
}