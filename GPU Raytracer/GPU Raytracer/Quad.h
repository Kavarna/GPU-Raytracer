#pragma once


#include "QuadShader.h"


ALIGN16 class Quad
{
public:
	struct SVertex
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 Texture;
		SVertex( float x, float y, float z, float u, float v ) :
			Position( x, y, z, 1.0f ), Texture( u, v ) {};
	};
private:

	MicrosoftPointer( ID3D11Buffer ) mVertexBuffer;
	MicrosoftPointer( ID3D11Buffer ) mIndexBuffer;

	std::vector<SVertex> mVertices;
	std::vector<DWORD> mIndices;

	std::shared_ptr<QuadShader> mShader;

	bool mSetOrtho = true;

	int mWidth, mHalfWidth;
	int mHeight, mHalfHeight;

	MicrosoftPointer( ID3D11Device ) mDevice;
	MicrosoftPointer( ID3D11DeviceContext ) mContext;
public:
	Quad( MicrosoftPointer( ID3D11Device ) Device, MicrosoftPointer( ID3D11DeviceContext ) Context,
		std::shared_ptr<QuadShader> Shader, int, int );
	~Quad( );
public:
	void __vectorcall Render( DirectX::FXMMATRIX& OrthoMatrix, ID3D11ShaderResourceView * Texture );
public:
	inline void * operator new ( size_t size )
	{
		return _aligned_malloc( size,16 );
	};
	inline void operator delete ( void * block )
	{
		_aligned_free( block );
	};
};

