Texture2D<float4> tex;
RWTexture2D<float4> rt;

SamplerState linear_sampler;

// Source: https://www.gamedev.net/forums/topic/616327-direct-compute-updown-scaling/
[numthreads(8, 8, 1)]
void main(uint3 gid: SV_GroupID, uint3 tid: SV_DispatchThreadID,
          uint3 gtid: SV_GroupThreadID)
{
    uint2 sample_pos = gid.xy * uint2(8, 8) + gtid.xy;

    uint2 texture_size = uint2(0, 0);
    rt.GetDimensions(texture_size.x, texture_size.y);

    [branch]
    if (sample_pos.x < texture_size.x && sample_pos.y < texture_size.y)
    {
        float2 uv = (sample_pos + 0.5f) / texture_size;
        rt[sample_pos] = tex.SampleLevel(linear_sampler, uv, 0);
    }
    else
        rt[sample_pos] = float4(0.0f, 0.0f, 0.0f, 1.0f);
}
