#pragma once

#include "Font.h"
#include "C2DShader.h"

class CText sealed
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;

	std::shared_ptr<CFont> mFont;
	std::shared_ptr<C2DShader> mShader;

	UINT mWindowWidth;
	UINT mWindowHeight;

	float mLastX = -1;
	float mLastY = -1;
	LPSTR mLastText = "";

	UINT mSize;

	UINT mIndexCount = 0;
	UINT mVertexCount = 0;

	ID3D11Device * mDevice;
	ID3D11DeviceContext * mContext;

public:
	CText( ID3D11Device * Device, ID3D11DeviceContext * Context,
		std::shared_ptr<C2DShader> Shader, std::shared_ptr<CFont> Font, 
		UINT WindowWidth, UINT WindowHeight,
		UINT MaxLength = 128);
	~CText( );
public:
	void Render( DirectX::FXMMATRIX& Projection, LPCSTR Text, float X, float Y,
		DirectX::XMFLOAT4 Color = DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	void ForceUpdate( );
private:
	void InitializeBuffers( UINT length );
	void Render( DirectX::FXMMATRIX& Projection, DirectX::XMFLOAT4& Color );
public:
	inline std::shared_ptr<CFont>& GetFont( )
	{
		return mFont;
	};
};

