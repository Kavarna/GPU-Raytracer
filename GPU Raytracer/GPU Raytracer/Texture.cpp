#include "Texture.h"

#include "WICTextureLoader.h"


CTexture::CTexture( LPWSTR lpPath, ID3D11Device* device, ID3D11DeviceContext* context )
{
	DX::ThrowIfFailed(
		CreateWICTextureFromFile(device, context, lpPath,
								 reinterpret_cast<ID3D11Resource**>(mTexture2D.ReleaseAndGetAddressOf()), &mTextureSRV)
	);
}


CTexture::~CTexture( )
{
	mTextureSRV.Reset( );
}
