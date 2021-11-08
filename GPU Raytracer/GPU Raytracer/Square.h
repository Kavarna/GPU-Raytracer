#pragma once

#include "commonincludes.h"
#include "C2DShader.h"
#include "Texture.h"


ALIGN16 class Square sealed
{
public:
	struct SVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
		SVertex( DirectX::XMFLOAT2 Position, DirectX::XMFLOAT2 Tex )
			: Position( Position.x, Position.y, 0.0f ), Tex( Tex )
		{ };
	};

private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> mVertBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;

	std::shared_ptr<C2DShader> mShader;

	std::unique_ptr<CTexture> mTexture;

	UINT mWindowWidth;
	UINT mWindowHeight;
	UINT mWidth;
	UINT mHeight;

	UINT mIndexCount;
	UINT mVertexCount;

	float mLastX;
	float mLastY;

	DirectX::XMMATRIX mWorld;

	ID3D11Device* mDevice;
	ID3D11DeviceContext* mContext;
public:
	Square( ID3D11Device * device, ID3D11DeviceContext * context,
		std::shared_ptr<C2DShader> Shader,
		UINT WindowWidth, UINT WindowHeight, UINT Width, UINT Height,
		LPWSTR Path = L"");
	~Square( );
public:
	void Render( DirectX::FXMMATRIX& Projection );
	void SetTexture( ID3D11ShaderResourceView* Tex );
private:
	void CreateBuffers( );
public:
	inline UINT GetWidth( ) { return mWidth; };
	inline UINT GetHeight( ) { return mHeight; };
	inline void Identity( ) { mWorld = DirectX::XMMatrixIdentity( ); };
	inline void Rotate( float Angle ) { mWorld *= DirectX::XMMatrixRotationZ( Angle ); };
	inline void Scale( float scale ) { mWorld *= DirectX::XMMatrixScaling( scale, scale, 1 ); };
	inline void Translate( float X, float Y ) { mWorld *= DirectX::XMMatrixTranslation( X, Y, 0 ); };
	inline void TranslateTo( float X, float Y )
	{
		mLastX = X;
		mLastY = Y;
		float halfWidth = ( float ) mWidth / 2.0f;
		float halfHeight = ( float ) mHeight / 2.0f;
		X += halfWidth;
		Y += halfHeight;
		float NewX = ( float( mWindowWidth / 2 ) * -1 + X );
		float NewY = ( float( mWindowHeight / 2 ) - Y );
		mWorld *= DirectX::XMMatrixTranslation( NewX, NewY, 0.0f );
	}
public:
	inline void* operator new (size_t size)
	{
		return _aligned_malloc( size, 16 );
	}
		inline void operator delete ( void* object )
	{
		return _aligned_free( object );
	}
	
};

