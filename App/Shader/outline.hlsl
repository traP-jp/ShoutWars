Texture2D    g_texture0 : register(t0);
SamplerState g_sampler0 : register(s0);

namespace s3d
{
    struct PSInput
    {
        float4 position : SV_POSITION;
        float4 color    : COLOR0;
        float2 uv       : TEXCOORD0;
    };
}

cbuffer PSConstants2D : register(b0)
{
    float4 g_colorAdd;
    float4 g_sdfParam;
    float4 g_sdfOutlineColor;
    float4 g_sdfShadowColor;
    float4 g_internal;
};

float GaussianWeight(int offset)
{
    if (offset == 0) return 0.38774;
    if (abs(offset) == 1) return 0.24477;
    return 0.06136;
}

// サンプリング間隔を広げて広い範囲をぼかす
float BlurAlpha(float2 uv, float2 texelSize)
{
    float result = 0.0;
    const float stepWidth = 4.0; // 1ステップあたり 4 ピクセル分飛ばしてサンプリング

    for (int y = -2; y <= 2; ++y)
    {
        for (int x = -2; x <= 2; ++x)
        {
            const float weight = GaussianWeight(x) * GaussianWeight(y);
            const float2 offset = float2(x, y) * texelSize * stepWidth;
            result += g_texture0.Sample(g_sampler0, uv + offset).a * weight;
        }
    }

    return result;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    uint width, height;
    g_texture0.GetDimensions(width, height);
    const float2 texelSize = 1.0 / float2(width, height);

    // 周囲のアルファをぼかして取得
    const float blurredAlpha = BlurAlpha(input.uv, texelSize);

    // グローの強さ (必要に応じて倍率を調整)
    const float outlineAlpha = saturate(blurredAlpha * 4.0);

    return float4(1.0, 1.0, 1.0, outlineAlpha) * input.color + g_colorAdd;
}
