
cbuffer cbPerObject : register( b0 )
{
    float4x4 g_WorldProjection;
}

struct VSOut
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VSOut main( float4 pos : POSITION, float2 tex : TEXCOORD )
{
    VSOut output = ( VSOut ) 0;
    output.Position = mul( pos, g_WorldProjection );
    output.TexCoord = tex;
    return output;
}