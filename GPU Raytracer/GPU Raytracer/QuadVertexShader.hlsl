
cbuffer cbPerWindow
{
    float4x4 OrthoMatrix;
};

struct VSOut
{
    float4 Position : SV_POSITION;
    float2 Texture : TEXCOORD;
};

VSOut main( float4 pos : POSITION, float2 tex : TEXCOORD )
{
    VSOut output;
    output.Position = mul( pos, OrthoMatrix );
    output.Texture = tex;
    return output;
}