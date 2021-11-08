#pragma once

#include "commonincludes.h"
#include "Texture.h"


class CFont sealed
{
public:
	struct SVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Texture;
	};
	struct SLetter
	{
		short ID;
		float u; // (0-1) - Start U
		float v; // (0-1) - Start V
		float width; // Letter width in texels
		float height; // Letter height in texels
		float xOffset; 
		float yOffset; 
		float xAdvance; // Advance X pixels when rendering
		float swidth; // Letter width in pixels
		float sheight; // Letter height in pixels
	};
	struct SKerning
	{
		int first; // First character
		int second; // Second character
		int amount; // Pixels
	};
private:
	std::unique_ptr<CTexture> mFPSTexture;
	std::unordered_map<size_t,SLetter*> mLetters;
	std::vector<SKerning> mKernings;
	std::array<int, 4> mPadding;
	std::wstring mName;
	int mSize;
	int mLineHeight;
	int mFPSTextureWidth;
	int mFPSTextureHeight;

	ID3D11Device * mDevice;
	ID3D11DeviceContext * mContext;
public:
	CFont( ID3D11Device * Device, ID3D11DeviceContext * Context, LPWSTR lpPath  );
	~CFont( );
public:
	CTexture * GetTexture( ) { return mFPSTexture.get( ); };
	void Build( void* Vertices, void* Indices,
		UINT& VertexCount, UINT& IndexCount,
		LPCSTR Message, float x, float y );
	float EstimateWidth( LPCSTR szText );
public:
	inline int GetHeight( ) { return mLineHeight; };
private:
	void ReadFile( LPWSTR );
	int GetKerning( int first, int second );
};

