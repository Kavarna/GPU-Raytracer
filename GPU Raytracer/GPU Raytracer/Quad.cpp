#include "Quad.h"



Quad::Quad( MicrosoftPointer( ID3D11Device ) Device, MicrosoftPointer( ID3D11DeviceContext ) Context,
	std::shared_ptr<QuadShader> Shader, int Width, int Height ) :
	mDevice( Device ),
	mContext( Context ),
	mShader( Shader ),
	mWidth( Width ),
	mHeight( Height )
{
	try
	{
		mHalfWidth = Width / 2;
		mHalfHeight = Height / 2;

		mVertices = std::vector<SVertex>
		{
			SVertex( ( float ) -mHalfWidth, ( float ) mHalfHeight,0.0f,0.0f,0.0f ),
			SVertex( ( float ) mHalfWidth,( float ) mHalfHeight, 0.0f,1.0f,0.0f ),
			SVertex( ( float ) mHalfWidth,( float ) -mHalfHeight,0.0f,1.0f,1.0f ),
			SVertex( ( float ) -mHalfWidth,( float ) -mHalfHeight,0.0f,0.0f,1.0f ),
		};

		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mVertexBuffer,
			D3D11_USAGE::D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER,
			sizeof( SVertex ) * 4, 0, ( void* ) &mVertices[ 0 ]
			);

		mIndices = std::vector<DWORD>
		{
			0, 1, 2,
			0, 2, 3
		};
		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mIndexBuffer,
			D3D11_USAGE::D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER,
			sizeof( DWORD ) * 6, 0, ( void* ) &mIndices[ 0 ]
			);
	}
	CATCH;
}


Quad::~Quad( )
{
	mVertexBuffer.Reset( );
	mIndexBuffer.Reset( );

	mShader.reset( );

	mDevice.Reset( );
	mContext.Reset( );
}

void Quad::Render( DirectX::FXMMATRIX& ortho, ID3D11ShaderResourceView * SRV )
{
	static UINT Stride = sizeof( SVertex );
	static UINT Offset = 0;
	mContext->IASetIndexBuffer( mIndexBuffer.Get( ), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0 );
	mContext->IASetVertexBuffers( 0, 1, mVertexBuffer.GetAddressOf( ), &Stride, &Offset );
	mContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	mShader->Render( mIndices.size( ), ortho, SRV );
}