
Texture2D Texture : register( t0 );
SamplerState WrapSampler : register( s0 );
 

cbuffer cbColor : register( b0 )
{
    float4 gColor;
    bool hasTexture;
}

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 main( PSIn input ) : SV_TARGET
{
    float4 TextureColor;
	if (hasTexture)
    {
        TextureColor = Texture.Sample( WrapSampler, input.TexCoord );
        clip( TextureColor.a - 0.10f );
    }
	else
    {
        TextureColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
    }
    return TextureColor * gColor;
}