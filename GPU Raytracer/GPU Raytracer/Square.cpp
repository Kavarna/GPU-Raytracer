#include "Square.h"



Square::Square( ID3D11Device * device, ID3D11DeviceContext * context,
	std::shared_ptr<C2DShader> Shader,
	UINT WindowWidth, UINT WindowHeight, UINT Width, UINT Height,
	LPWSTR Path ) :
	mDevice( device ),
	mContext( context ),
	mWindowWidth( WindowWidth ),
	mWindowHeight( WindowHeight ),
	mWidth( Width ),
	mHeight( Height ),
	mShader( Shader ),
	mLastX( ( float ) WindowWidth / 2.0f ),
	mLastY( ( float ) WindowHeight / 2.0f )
{
	try
	{
		CreateBuffers( );
		mWorld = DirectX::XMMatrixIdentity( );
		if ( Path != L"" )
			mTexture = std::make_unique<CTexture>( Path, mDevice, mContext );
	}
	CATCH;
}

Square::~Square( )
{
	mShader.reset( );
	mVertBuffer.Reset( );
	mIndexBuffer.Reset( );
}

void Square::CreateBuffers( )
{
	float halfWidth = ( float ) mWidth / 2.0f;
	float halfHeight = ( float ) mHeight / 2.0f;
	SVertex Vertices[ ] =
	{
		SVertex( DirectX::XMFLOAT2( -halfWidth,halfHeight ),DirectX::XMFLOAT2( 0.0f,0.0f ) ),
		SVertex( DirectX::XMFLOAT2( halfWidth,halfHeight ), DirectX::XMFLOAT2( 1.0f,0.0f ) ),
		SVertex( DirectX::XMFLOAT2( halfWidth,-halfHeight ), DirectX::XMFLOAT2( 1.0f,1.0f ) ),
		SVertex( DirectX::XMFLOAT2( -halfWidth,-halfHeight ), DirectX::XMFLOAT2( 0.0f,1.0f ) ),
	};
	mVertexCount = ARRAYSIZE( Vertices );
	ZeroMemoryAndDeclare( D3D11_BUFFER_DESC, buffDesc );
	ZeroMemoryAndDeclare( D3D11_SUBRESOURCE_DATA, buffData );
	buffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	buffDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	buffDesc.ByteWidth = sizeof( SVertex ) * mVertexCount;
	buffData.pSysMem = Vertices;
	DX::ThrowIfFailed( mDevice->CreateBuffer( &buffDesc, &buffData, &mVertBuffer ) );

	DWORD Indices[ ] =
	{
		0,1,2,
		0,2,3,
	};
	mIndexCount = ARRAYSIZE( Indices );
	ZeroVariable( buffDesc );
	ZeroVariable( buffData );
	buffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	buffDesc.ByteWidth = sizeof( DWORD ) * mIndexCount;
	buffDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	buffData.pSysMem = Indices;
	DX::ThrowIfFailed( mDevice->CreateBuffer( &buffDesc, &buffData, &mIndexBuffer ) );
}

void Square::Render( DirectX::FXMMATRIX& Projection )
{
	static UINT Stride = sizeof( SVertex );
	static UINT Offset = 0;
	
	// Old toys
	ID3D11Buffer * oldVertexBuffer = nullptr;
	ID3D11Buffer * oldIndexBuffer = nullptr;
	UINT oldStride;
	UINT oldOffset;
	DXGI_FORMAT oldIndexFormat;
	UINT oldIOffset;
	D3D11_PRIMITIVE_TOPOLOGY oldTopology;
	mContext->IAGetVertexBuffers( 0, 1, &oldVertexBuffer, &oldStride, &oldOffset );
	mContext->IAGetIndexBuffer( &oldIndexBuffer, &oldIndexFormat, &oldIOffset );
	mContext->IAGetPrimitiveTopology( &oldTopology );

	// New toys
	mContext->RSSetState( DX::NoCulling.Get( ) );
	mContext->IASetVertexBuffers( 0, 1, mVertBuffer.GetAddressOf( ), &Stride, &Offset );
	mContext->IASetIndexBuffer( mIndexBuffer.Get( ), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0 );
	mContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	mShader->Render( mIndexCount, mWorld, Projection, mTexture.get( ) );

	// Old toys
	mContext->IASetVertexBuffers( 0, 1, &oldVertexBuffer, &oldStride, &oldOffset );
	mContext->IASetIndexBuffer( oldIndexBuffer, oldIndexFormat, oldIOffset );
	mContext->IASetPrimitiveTopology( oldTopology );
}

void Square::SetTexture( ID3D11ShaderResourceView * SRV )
{
	if ( mTexture == nullptr )
	{
		mTexture = std::make_unique<CTexture>( );
	}
	mTexture->SetTexture( SRV );
}
