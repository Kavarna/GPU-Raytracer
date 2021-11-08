#include "Text.h"



CText::CText( ID3D11Device * device, ID3D11DeviceContext * context,
	std::shared_ptr<C2DShader> Shader, std::shared_ptr<CFont> Font,
	UINT WindowWidth, UINT WindowHeight,
	UINT maxLength ) :
	mDevice( device ),
	mContext( context ),
	mShader( Shader ),
	mFont( Font ),
	mWindowWidth( WindowWidth ),
	mWindowHeight( WindowHeight ),
	mSize(maxLength )
{
	try
	{
		InitializeBuffers( maxLength );
	}
	CATCH;
}


CText::~CText( )
{
	mFont.reset( );
	mShader.reset( );
}


void CText::InitializeBuffers( UINT length )
{
	ShaderHelper::CreateBuffer( mDevice, mVertexBuffer.GetAddressOf( ),
		D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER,
		sizeof( CFont::SVertex ) * 4 * length, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE );
	ShaderHelper::CreateBuffer( mDevice, mIndexBuffer.GetAddressOf( ),
		D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER,
		sizeof( DWORD ) * 6 * length, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE );
}


void CText::Render( DirectX::FXMMATRIX& Matrix, LPCSTR Text, float X, float Y, DirectX::XMFLOAT4 Color )
{
	if ( X == mLastX && Y == mLastY &&
		(strcmp( mLastText, Text ) != 0) )
	{
		Render( Matrix, Color );
		return;
	}
	mLastX = X;
	mLastY = Y;
	mLastText = const_cast< LPSTR >( Text );

	float CX, CY;
	CX = float( mWindowWidth ) / 2.0f * -1 + X;
	CY = float( mWindowHeight ) / 2.0f - Y;
	
	D3D11_MAPPED_SUBRESOURCE Vertices, Indices;
	mContext->Map( mVertexBuffer.Get( ), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &Vertices );

	mContext->Map( mIndexBuffer.Get( ), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &Indices );

	mFont->Build( Vertices.pData, Indices.pData, mVertexCount, mIndexCount, Text, CX, CY );

	mContext->Unmap( mIndexBuffer.Get( ), 0 );

	mContext->Unmap( mVertexBuffer.Get( ), 0 );

	Render( Matrix, Color );
}

void CText::ForceUpdate( )
{
	mLastX = -1;
	mLastY = -1;
}

void CText::Render( DirectX::FXMMATRIX& Matrix, DirectX::XMFLOAT4& Color )
{
	static UINT Stride = sizeof( CFont::SVertex );
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
	mContext->IASetVertexBuffers( 0, 1, mVertexBuffer.GetAddressOf( ), &Stride, &Offset );
	mContext->IASetIndexBuffer( mIndexBuffer.Get( ), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0 );
	mContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	mShader->Render( mIndexCount, DirectX::XMMatrixIdentity( ), Matrix, mFont->GetTexture( ), Color );

	// Old toys
	mContext->IASetVertexBuffers( 0, 1, &oldVertexBuffer, &oldStride, &oldOffset );
	mContext->IASetIndexBuffer( oldIndexBuffer, oldIndexFormat, oldIOffset );
	mContext->IASetPrimitiveTopology( oldTopology );
}