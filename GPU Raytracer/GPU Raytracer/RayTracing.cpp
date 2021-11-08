#include "RayTracing.h"

#include "DDSTextureLoader.h"


RayTracing::RayTracing( MicrosoftPointer( ID3D11Device ) Device, MicrosoftPointer( ID3D11DeviceContext ) Context,
	UINT Width, UINT Height) :
	mDevice( Device ),
	mContext( Context )
{
	try
	{
		ID3D11ComputeShader ** CS = &mComputeShader;
		ShaderHelper::CreateShaderFromFile(
			L"Shaders/RayTracingComputeShader.cso", "cs_5_0",
			mDevice.Get( ), &mBlobs[ 0 ], reinterpret_cast< ID3D11DeviceChild** >( CS )
		);

		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mCameraBuffer,
			D3D11_USAGE::D3D11_USAGE_DYNAMIC,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
			sizeof( SCamera ),
			D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE
		);

		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mOtherInfoBuffer,
			D3D11_USAGE::D3D11_USAGE_DEFAULT,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
			sizeof( SOtherInfo ),0
		);

		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mModelsBuffer,
			D3D11_USAGE::D3D11_USAGE_DYNAMIC,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
			sizeof( SModels ), D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE
		);

		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mLightsBuffer,
			D3D11_USAGE::D3D11_USAGE_DEFAULT,
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
			sizeof(SLights),0
		);

		ZeroMemoryAndDeclare( D3D11_TEXTURE2D_DESC, texDesc );
		texDesc.ArraySize = 1;
		texDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE |
			D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
		texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.Width = Width;
		texDesc.Height = Height;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		DX::ThrowIfFailed(
			mDevice->CreateTexture2D(
				&texDesc, nullptr, &mTexture
			)
		);
		ZeroMemoryAndDeclare( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		DX::ThrowIfFailed(
			mDevice->CreateShaderResourceView(
				mTexture.Get( ), &srvDesc, &mTextureSRV
			)
		);
		ZeroMemoryAndDeclare( D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDesc );
		uavDesc.Format = texDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		DX::ThrowIfFailed(
			mDevice->CreateUnorderedAccessView(
				mTexture.Get( ), &uavDesc, &mTextureUAV
			)
		);
		ZeroMemoryAndDeclare( D3D11_RENDER_TARGET_VIEW_DESC, rtvDesc );
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		DX::ThrowIfFailed(
			mDevice->CreateRenderTargetView(
				mTexture.Get( ), &rtvDesc, &mTextureRTV
			)
		);

		DX::ThrowIfFailed(
			DirectX::CreateDDSTextureFromFile(
				mDevice.Get(),
				L"Data/Skymap.dds",
				reinterpret_cast<ID3D11Resource**>(mBackground.GetAddressOf()),
				&mBackgroundSRV
			)
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


RayTracing::~RayTracing( )
{
	for ( auto & iter : mBlobs )
	{
		iter.Reset( );
	}
	mComputeShader.Reset( );
	mTexture.Reset( );
	mTextureSRV.Reset( );
	mTextureUAV.Reset( );
}


void RayTracing::UpdateOtherInfo( RayTracing::SOtherInfo const& info )
{
	mContext->UpdateSubresource(
		mOtherInfoBuffer.Get( ),
		0, nullptr,
		&info, 0, 0
	);
	mInfo = info;
}

void RayTracing::UpdateLights( RayTracing::SLights const& lights )
{
	mContext->UpdateSubresource(
		mLightsBuffer.Get( ),
		0, nullptr,
		&lights, 0, 0
	);
}

void RayTracing::Trace( RayTracing::SCamera const& Camera)
{
	static D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	mContext->CSSetShader( mComputeShader.Get( ), 0, 0 );
	DX::ThrowIfFailed(
		mContext->Map(
			mCameraBuffer.Get( ),
			0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
			0, &MappedSubresource
		)
	);
	( ( SCamera* ) MappedSubresource.pData )->BottomLeftCorner = Camera.BottomLeftCorner;
	( ( SCamera* ) MappedSubresource.pData )->CamPos = Camera.CamPos;
	( ( SCamera* ) MappedSubresource.pData )->TopLeftCorner = Camera.TopLeftCorner;
	( ( SCamera* ) MappedSubresource.pData )->TopRightCorner = Camera.TopRightCorner;
	mContext->Unmap( mCameraBuffer.Get( ), 0 );
	mContext->CSSetConstantBuffers( 0, 1, mCameraBuffer.GetAddressOf( ) );
	mContext->CSSetConstantBuffers( 1, 1, mOtherInfoBuffer.GetAddressOf( ) );

	DX::ThrowIfFailed(
		mContext->Map(
			mModelsBuffer.Get( ),
			0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
			0, &MappedSubresource
		)
	);
	( ( SModels* ) MappedSubresource.pData )->NumSpheres = mModels.NumSpheres;
	memcpy( ( ( SModels* ) MappedSubresource.pData )->Spheres, mModels.Spheres, sizeof( SSphere ) * mModels.NumSpheres );
	( ( SModels* ) MappedSubresource.pData )->NumTriangles = mModels.NumTriangles;
	memcpy( ( ( SModels* ) MappedSubresource.pData )->Triangles, mModels.Triangles, sizeof( STriangle ) * mModels.NumTriangles );
	mContext->Unmap( mModelsBuffer.Get( ), 0 );
	mContext->CSSetConstantBuffers( 2, 1, mModelsBuffer.GetAddressOf( ) );
	mContext->CSSetConstantBuffers( 3, 1, mLightsBuffer.GetAddressOf( ) );

	mContext->CSSetSamplers( 0, 1, mWrapSampler.GetAddressOf( ) );
	mContext->CSSetShaderResources( 0, 1, mBackgroundSRV.GetAddressOf( ) );
	mContext->CSSetUnorderedAccessViews( 0, 1, mTextureUAV.GetAddressOf( ), nullptr );

	unsigned int NumHorizontalGroups = ( unsigned int ) ceil( ( float ) mInfo.Width / 16.f );
	unsigned int NumVerticalGroups = ( unsigned int ) ceil( ( float ) mInfo.Height / 16.f );
	mContext->Dispatch( NumHorizontalGroups, NumVerticalGroups, 1 );

	mContext->CSSetShader( 0, 0, 0 );
	ID3D11UnorderedAccessView * UAVs = { nullptr };
	mContext->CSSetUnorderedAccessViews( 0, 1, &UAVs, nullptr );
}