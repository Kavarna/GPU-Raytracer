#include "C2DShader.h"



C2DShader::C2DShader( ID3D11Device * Device, ID3D11DeviceContext * Context ) :
	mDevice( Device ),
	mContext( Context )
{
	try
	{
		ID3D11VertexShader ** VS = &mVertexShader;
		ID3D11PixelShader ** PS = &mPixelShader;
		ShaderHelper::CreateShaderFromFile( L"Shaders/2DVertexShader.cso", "vs_4_0",
			mDevice, &mBlobs[ 0 ], reinterpret_cast< ID3D11DeviceChild** >( VS ) );
		ShaderHelper::CreateShaderFromFile( L"Shaders/2DPixelShader.cso", "ps_4_0",
			mDevice, &mBlobs[ 1 ], reinterpret_cast< ID3D11DeviceChild** >( PS ) );
		D3D11_INPUT_ELEMENT_DESC layout[ 2 ];
		layout[ 0 ].AlignedByteOffset = 0;
		layout[ 0 ].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		layout[ 0 ].InputSlot = 0;
		layout[ 0 ].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		layout[ 0 ].InstanceDataStepRate = 0;
		layout[ 0 ].SemanticIndex = 0;
		layout[ 0 ].SemanticName = "POSITION";
		layout[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // 12 bytes
		layout[ 1 ].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
		layout[ 1 ].InputSlot = 0;
		layout[ 1 ].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		layout[ 1 ].InstanceDataStepRate = 0;
		layout[ 1 ].SemanticIndex = 0;
		layout[ 1 ].SemanticName = "TEXCOORD";
		UINT layoutCount = ARRAYSIZE( layout );
		DX::ThrowIfFailed( 
			mDevice->CreateInputLayout( layout, layoutCount,
				mBlobs[0]->GetBufferPointer(), mBlobs[0]->GetBufferSize(), &mInputLayout )
			);
		ShaderHelper::CreateBuffer(
			mDevice,&mPerObjectBuffer,D3D11_USAGE::D3D11_USAGE_DYNAMIC,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,sizeof(SPerObject ),
			D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE
			);
		ShaderHelper::CreateBuffer(
			mDevice, &mColorBuffer, D3D11_USAGE::D3D11_USAGE_DYNAMIC,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER, sizeof( SColor ),
			D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE
			);
		ZeroMemoryAndDeclare( D3D11_SAMPLER_DESC, sampDesc );
		sampDesc.AddressU =
			sampDesc.AddressV =
			sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		sampDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.MaxAnisotropy = 16;
		sampDesc.MaxLOD = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MipLODBias = 3;
		DX::ThrowIfFailed(
			mDevice->CreateSamplerState( &sampDesc, &mSampler )
			);
	}
	CATCH;
}


C2DShader::~C2DShader( )
{
	for ( size_t i = 0; i < mBlobs.size( ); ++i )
		mBlobs[ i ].Reset( );
	mVertexShader.Reset( );
	mPixelShader.Reset( );
	mInputLayout.Reset( );
}


void C2DShader::Render( UINT IndexCount, DirectX::FXMMATRIX& World, DirectX::FXMMATRIX& Projection,
	CTexture * Texture, DirectX::XMFLOAT4 Color )
{
	// Old toys addresses
	ID3D11VertexShader * oldVS = nullptr;
	ID3D11PixelShader * oldPS = nullptr;
	ID3D11InputLayout * oldLayout = nullptr;
	ID3D11SamplerState * oldSampler = nullptr;
	ID3D11Buffer * oldBuffer = nullptr;
	ID3D11Buffer * oldPSBuffer = nullptr;
	ID3D11ShaderResourceView * oldSRV = nullptr;

	// Get the old toys
	mContext->IAGetInputLayout( &oldLayout );
	mContext->VSGetShader( &oldVS, nullptr, nullptr );
	mContext->PSGetShader( &oldPS, nullptr, nullptr );
	mContext->PSGetSamplers( 0, 1, &oldSampler );
	mContext->VSGetConstantBuffers( 0, 1, &oldBuffer );
	mContext->PSGetConstantBuffers( 0, 1, &oldPSBuffer );
	mContext->PSGetShaderResources( 0, 1, &oldSRV );

	// Replace the old toys
	mContext->IASetInputLayout( mInputLayout.Get() );
	mContext->VSSetShader( mVertexShader.Get( ), nullptr, 0 );
	mContext->PSSetShader( mPixelShader.Get( ), nullptr, 0 );
	mContext->PSSetSamplers( 0, 1, mSampler.GetAddressOf( ) );

	D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	DirectX::XMMATRIX WVP = DirectX::XMMatrixTranspose( World * Projection );

	mContext->Map( mPerObjectBuffer.Get( ), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource );
	
	( ( SPerObject* ) MappedSubresource.pData )->WP = WVP;

	mContext->Unmap( mPerObjectBuffer.Get( ), 0 );
	mContext->VSSetConstantBuffers( 0, 1, mPerObjectBuffer.GetAddressOf( ) );

	mContext->Map( mColorBuffer.Get( ), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource );

	( ( SColor* ) MappedSubresource.pData )->Color = Color;
	( ( SColor* ) MappedSubresource.pData )->HasTexture = ( Texture == nullptr || Texture == 0 ) ? FALSE : TRUE;

	mContext->Unmap( mColorBuffer.Get( ), 0 );
	mContext->PSSetConstantBuffers( 0, 1, mColorBuffer.GetAddressOf( ) );

	if ( !( Texture == nullptr || Texture == 0 ) )
	{
		ID3D11ShaderResourceView* SRV = Texture->GetTexture( );
		mContext->PSSetShaderResources( 0, 1, &SRV );
	}

	mContext->DrawIndexed( IndexCount, 0, 0 );
	mContext->OMSetDepthStencilState( nullptr, 0 );

	// Place back the old toys
	mContext->IASetInputLayout( oldLayout );
	mContext->VSSetShader( oldVS, nullptr, 0 );
	mContext->PSSetShader( oldPS, nullptr, 0 );
	mContext->PSSetSamplers( 0, 1, &oldSampler );
	mContext->VSSetConstantBuffers( 0, 1, &oldBuffer );
	mContext->PSSetConstantBuffers( 0, 1, &oldPSBuffer );
	mContext->PSSetShaderResources( 0, 1, &oldSRV );
}