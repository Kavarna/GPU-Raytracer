#pragma once

#include "ShaderHelper.h"


class QuadShader
{
public:
	struct SVSPerWindow
	{
		DirectX::XMMATRIX OrthoMatrix;
	};
private:
	MicrosoftPointer( ID3D11VertexShader ) mVertexShader;
	MicrosoftPointer( ID3D11PixelShader ) mPixelShader;
	MicrosoftPointer( ID3D11InputLayout ) mLayout;
	std::array<MicrosoftPointer( ID3DBlob ), 2> mBlobs;

	MicrosoftPointer( ID3D11Buffer ) mVSPerWindowBuffer;
	MicrosoftPointer( ID3D11SamplerState ) mWrapSampler;


	MicrosoftPointer( ID3D11Device ) mDevice;
	MicrosoftPointer( ID3D11DeviceContext ) mContext;
public:
	QuadShader( MicrosoftPointer( ID3D11Device ) Device, MicrosoftPointer( ID3D11DeviceContext ) Context );
	~QuadShader( );
public:
	void __vectorcall Render( UINT IndexCount, DirectX::FXMMATRIX& Ortho, ID3D11ShaderResourceView * SRV );
};

