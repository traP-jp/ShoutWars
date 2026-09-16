// 背景とキャラに、画面全体の仕上げをかける (Source/PostProcess.cpp から使う)

Texture2D    g_texture0 : register(t0); // 仕上げる前の画面
Texture2D    g_texture1 : register(t1); // 近くをにじませる光
Texture2D    g_texture2 : register(t2); // 広く漂う光
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

cbuffer PostProcess : register(b1)
{
    float g_grainSeed;
};

// 夜の背景や灰色の地面は光らせず、月・弾・溜めの光のような明るいところだけを光らせる
static const float BrightThreshold = 0.8;
static const float NearGlowStrength = 0.35;
static const float WideGlowStrength = 0.2;

static const float Contrast = 1.08;
// 暗いところを青に、明るいところを暖色に寄せ、平たい灰色に色味を足す
static const float3 ShadowTint = float3(-0.02, 0.0, 0.03);
static const float3 HighlightTint = float3(0.03, 0.01, -0.02);

static const float VignetteStrength = 0.4;
static const float GrainStrength = 0.035;

float4 PS_BrightPass(s3d::PSInput input) : SV_TARGET
{
    const float3 color = g_texture0.Sample(g_sampler0, input.uv).rgb;
    const float brightness = max(color.r, max(color.g, color.b));
    return float4(color * smoothstep(BrightThreshold, 1.0, brightness), 1.0);
}

float3 Grade(float3 color)
{
    color = saturate((color - 0.5) * Contrast + 0.5);
    const float luminance = dot(color, float3(0.299, 0.587, 0.114));
    return saturate(color + lerp(ShadowTint, HighlightTint, luminance));
}

float Vignette(float2 uv)
{
    const float2 fromCenter = (uv - 0.5) * float2(16.0 / 9.0, 1.0);
    return 1.0 - VignetteStrength * smoothstep(0.45, 1.1, length(fromCenter));
}

// フレームごとに変わる、ピクセルごとの -0.5〜0.5 の乱数
float Grain(float2 position)
{
    return frac(sin(dot(position + g_grainSeed, float2(12.9898, 78.233))) * 43758.5453) - 0.5;
}

float4 Finish(s3d::PSInput input, float3 glow)
{
    const float3 color = g_texture0.Sample(g_sampler0, input.uv).rgb + glow;
    const float3 finished = Grade(color) * Vignette(input.uv) + GrainStrength * Grain(input.position.xy);
    return float4(saturate(finished), 1.0);
}

float4 PS_Finish(s3d::PSInput input) : SV_TARGET
{
    return Finish(input, 0.0);
}

float4 PS_FinishWithBloom(s3d::PSInput input) : SV_TARGET
{
    const float3 glow = NearGlowStrength * g_texture1.Sample(g_sampler0, input.uv).rgb
        + WideGlowStrength * g_texture2.Sample(g_sampler0, input.uv).rgb;
    return Finish(input, glow);
}
