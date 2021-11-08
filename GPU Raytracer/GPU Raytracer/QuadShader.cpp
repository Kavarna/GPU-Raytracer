#include "QuadShader.h"



QuadShader::QuadShader( MicrosoftPointer( ID3D11Device ) Device, MicrosoftPointer( ID3D11DeviceContext ) Context ) :
	mDevice( Device ),
	mContext( Context )
{
	try
	{
		ID3D11VertexShader ** VS = &mVertexShader;
		ID3D11PixelShader ** PS = &mPixelShader;
		ShaderHelper::CreateShaderFromFile(
			L"Shaders/QuadVertexShader.cso", "vs_4_0",
			mDevice.Get( ), &mBlobs[ 0 ],
			reinterpret_cast< ID3D11DeviceChild** >( VS )
			);

		D3D11_INPUT_ELEMENT_DESC layout[ 2 ];
		layout[ 0 ].AlignedByteOffset = 0;
		layout[ 0 ].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		layout[ 0 ].InputSlot = 0;
		layout[ 0 ].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		layout[ 0 ].InstanceDataStepRate = 0;
		layout[ 0 ].SemanticIndex = 0;
		layout[ 0 ].SemanticName = "POSITION";
		layout[ 1 ].AlignedByteOffset = 16;
		layout[ 1 ].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
		layout[ 1 ].InputSlot = 0;
		layout[ 1 ].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		layout[ 1 ].InstanceDataStepRate = 0;
		layout[ 1 ].SemanticIndex = 0;
		layout[ 1 ].SemanticName = "TEXCOORD";
		int size = ARRAYSIZE( layout );
		DX::ThrowIfFailed(
			mDevice->CreateInputLayout(
				layout, size,
				mBlobs[ 0 ]->GetBufferPointer( ),
				mBlobs[ 0 ]->GetBufferSize( ),
				&mLayout
				)
			);

		ShaderHelper::CreateShaderFromFile(
			L"Shaders/QuadPixelShader.cso", "ps_4_0",
			mDevice.Get( ), &mBlobs[ 1 ],
			reinterpret_cast< ID3D11DeviceChild** > ( PS )
			);

		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mVSPerWindowBuffer,
			D3D11_USAGE::D3D11_USAGE_DYNAMIC,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
			sizeof( SVSPerWindow ), D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE
			);

		ZeroMemoryAndDeclare( D3D11_SAMPLER_DESC, sampDesc );
		sampDesc.AddressU =
			sampDesc.AddressV =
			sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
		sampDesc.MaxAnisotropy = 16;
		sampDesc.MaxLOD = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MipLODBias = 0;
		DX::ThrowIfFailed(
			mDevice->CreateSamplerState(
				&sampDesc, &mWrapSampler
				)
			);
	}
	CATCH;
}


QuadShader::~QuadShader( )
{
	for ( auto & iter : mBlobs )
	{
		if ( iter )
		{
			iter.Reset( );
			iter = nullptr;
		}
	}
	mVertexShader.Reset( );
	mPixelShader.Reset( );
	mLayout.Reset( );

	mVSPerWindowBuffer.Reset( );
	mWrapSampler.Reset( );

	mDevice.Reset( );
	mContext.Reset( );
}

void QuadShader::Render( UINT IndexCount, DirectX::FXMMATRIX& Ortho, ID3D11ShaderResourceView * SRV )
{
	static D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	mContext->IASetInputLayout( mLayout.Get( ) );
	mContext->VSSetShader( mVertexShader.Get( ), 0, 0 );
	mContext->PSSetShader( mPixelShader.Get( ), 0, 0 );
	mContext->PSSetSamplers( 0, 1, mWrapSampler.GetAddressOf( ) );
	mContext->PSSetShaderResources( 0, 1, &SRV );

	mContext->Map( mVSPerWindowBuffer.Get( ), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource );
	( ( SVSPerWindow* ) MappedSubresource.pData )->OrthoMatrix = DirectX::XMMatrixTranspose( Ortho );
	mContext->Unmap( mVSPerWindowBuffer.Get( ), 0 );
	mContext->VSSetConstantBuffers( 0, 1, mVSPerWindowBuffer.GetAddressOf( ) );

	mContext->DrawIndexed( IndexCount, 0, 0 );

	ID3D11ShaderResourceView * SRVs = { nullptr };
	mContext->PSSetShaderResources( 0, 1, &SRVs );
}