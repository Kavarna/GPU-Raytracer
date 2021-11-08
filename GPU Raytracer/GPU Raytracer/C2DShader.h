#pragma once

#include "commonincludes.h"
#include "ShaderHelper.h"
#include "Texture.h"

class C2DShader sealed
{
public:

	struct SPerObject
	{
		DirectX::XMMATRIX WP;
	};
	struct SColor
	{
		DirectX::XMFLOAT4 Color;
		BOOL HasTexture;
		DirectX::XMFLOAT3 Pad;
	};

private:

	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
	std::array < Microsoft::WRL::ComPtr<ID3DBlob>, 2> mBlobs;

	Microsoft::WRL::ComPtr<ID3D11Buffer> mPerObjectBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mColorBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mSampler;

	ID3D11Device * mDevice;
	ID3D11DeviceContext * mContext;
public:
	C2DShader( ID3D11Device * Device, ID3D11DeviceContext * Context );
	~C2DShader( );

public:
	void Render( UINT indexCount, DirectX::FXMMATRIX& World, DirectX::FXMMATRIX& Projection,
		CTexture * TextureSRV = nullptr,
		DirectX::XMFLOAT4 Color = DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) );
};

