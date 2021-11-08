#pragma once


#include "ShaderHelper.h"



class RayTracing
{
public: // To be modified in RayTracingHeader.hlsli also
	static constexpr const unsigned int MaxSpheres = 240;
	static constexpr const unsigned int MaxTriangles = 16;
	static constexpr const unsigned int MaxModels = 256; 
	static constexpr const unsigned int MaxPointLights = 2;
public:
	struct SSphere
	{
		DirectX::XMFLOAT3 Pos; // 12
		float Radius; // 4
		DirectX::XMFLOAT4 Color; // 16
		float Reflectivity; // 4
		DirectX::XMFLOAT3 Pad; // 12
	};
	struct STriangle // 80 bytes
	{
		DirectX::XMFLOAT3 A; // 12
		float pad0; // 4
		DirectX::XMFLOAT3 B; // 12
		float pad1; // 4
		DirectX::XMFLOAT3 C; // 12
		float pad2; // 4
		float Reflectivity; // 4
		DirectX::XMFLOAT3 pad3; // 12
		DirectX::XMFLOAT4 Color; // 16
	};
	struct SDirectionalLight
	{
		DirectX::XMFLOAT3 Direction;
		float pad;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Ambient;
	};
	struct SPointLight
	{
		DirectX::XMFLOAT3 Position;
		float pad;
		DirectX::XMFLOAT4 Diffuse;
	};
public:
	struct SCamera
	{
		DirectX::XMFLOAT4 CamPos;
		DirectX::XMFLOAT4 TopLeftCorner;
		DirectX::XMFLOAT4 TopRightCorner;
		DirectX::XMFLOAT4 BottomLeftCorner;
	};
	struct SOtherInfo
	{
		UINT Width;
		UINT Height;
		UINT MaxRecursion;
		float RayMaxLength;
	};
	struct SModels
	{
		UINT NumSpheres;
		UINT NumTriangles;
		DirectX::XMFLOAT2 Pad;
		SSphere Spheres[ MaxSpheres ];
		STriangle Triangles[ MaxTriangles ];
	};
	struct SLights
	{
		SDirectionalLight Sun;
		UINT NumPointLights;
		DirectX::XMFLOAT3 Pad;
		SPointLight PointLights[ MaxPointLights ];
	};
private:
	MicrosoftPointer( ID3D11ComputeShader ) mComputeShader;
	MicrosoftPointer( ID3D11Texture2D ) mTexture;
	MicrosoftPointer( ID3D11ShaderResourceView ) mTextureSRV;
	MicrosoftPointer( ID3D11UnorderedAccessView ) mTextureUAV;
	MicrosoftPointer( ID3D11RenderTargetView ) mTextureRTV;
	std::array<MicrosoftPointer( ID3DBlob ), 1> mBlobs;

	MicrosoftPointer( ID3D11Texture2D ) mBackground;
	MicrosoftPointer( ID3D11ShaderResourceView ) mBackgroundSRV;
	MicrosoftPointer( ID3D11SamplerState ) mWrapSampler;

	MicrosoftPointer( ID3D11Buffer ) mCameraBuffer;
	MicrosoftPointer( ID3D11Buffer ) mOtherInfoBuffer;
	MicrosoftPointer( ID3D11Buffer ) mModelsBuffer;
	MicrosoftPointer( ID3D11Buffer ) mLightsBuffer;

	SOtherInfo mInfo;
	SModels mModels;

	MicrosoftPointer( ID3D11Device ) mDevice;
	MicrosoftPointer( ID3D11DeviceContext ) mContext;
public:
	RayTracing( MicrosoftPointer( ID3D11Device ) Device, MicrosoftPointer( ID3D11DeviceContext ) Context,
		UINT Width, UINT Height );
	~RayTracing( );
public:
	void UpdateOtherInfo( SOtherInfo const& );
	void UpdateLights( SLights const& );
	void Trace( SCamera const& Camera );
public:
	inline ID3D11RenderTargetView * GetTextureAsRTV( ) const
	{
		return mTextureRTV.Get( );
	}
	inline ID3D11ShaderResourceView * GetTextureAsSRV( ) const
	{
		return mTextureSRV.Get( );
	};
	inline SModels* GetModelsRW( )
	{
		return &mModels;
	};
};