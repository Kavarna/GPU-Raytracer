#pragma once

#include "commonincludes.h"



class CTexture
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mTexture2D;
public:
	CTexture( ) = default;
	CTexture( LPWSTR lpPath, ID3D11Device *, ID3D11DeviceContext * );
	~CTexture( );
public:
	ID3D11ShaderResourceView* GetTexture() const
	{
		return mTextureSRV.Get( );
	}
	void SetTexture( ID3D11ShaderResourceView * newSRV )
	{
		mTextureSRV.Reset( );
		mTextureSRV = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>( newSRV );
	}
};

