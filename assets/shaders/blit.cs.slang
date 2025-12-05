Texture2D<float4> tex;

RWTexture2D<float4> rt;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(int2 thread_id: SV_DispatchThreadID)
{
    float4 color = tex[thread_id.xy].rgba;

    rt[thread_id.xy] = color;
}