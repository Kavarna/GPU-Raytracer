#pragma once

#include "commonincludes.h"


namespace ShaderHelper
{

	inline void CreateShaderFromFile( LPWSTR filePath, LPSTR profile,
		ID3D11Device * device, ID3DBlob ** ShaderBlob, ID3D11DeviceChild ** Shader )
	{

		DX::ThrowIfFailed( D3DReadFileToBlob( filePath, ShaderBlob ) );

		switch ( profile[ 0 ] )
		{
		case 'v': // Vertex Shader
		DX::ThrowIfFailed(
			device->CreateVertexShader( ( *ShaderBlob )->GetBufferPointer( ), ( *ShaderBlob )->GetBufferSize( ), nullptr,
				reinterpret_cast< ID3D11VertexShader** >( Shader ) )
			);
		break;
		case 'h': // Hull shader
		DX::ThrowIfFailed(
			device->CreateHullShader( ( *ShaderBlob )->GetBufferPointer( ), ( *ShaderBlob )->GetBufferSize( ), nullptr,
				reinterpret_cast< ID3D11HullShader** > ( Shader ) )
			);
		break;
		case 'd': // Domain Shader
		DX::ThrowIfFailed(
			device->CreateDomainShader( ( *ShaderBlob )->GetBufferPointer( ), ( *ShaderBlob )->GetBufferSize( ), nullptr,
				reinterpret_cast< ID3D11DomainShader** > ( Shader ) )
			);
		break;
		case 'g': // Geometry Shader
		DX::ThrowIfFailed(
			device->CreateGeometryShader( ( *ShaderBlob )->GetBufferPointer( ), ( *ShaderBlob )->GetBufferSize( ), nullptr,
				reinterpret_cast< ID3D11GeometryShader** > ( Shader ) )
			);
		break;
		case 'p': // Pixel Shader
		DX::ThrowIfFailed(
			device->CreatePixelShader( ( *ShaderBlob )->GetBufferPointer( ), ( *ShaderBlob )->GetBufferSize( ), nullptr,
				reinterpret_cast< ID3D11PixelShader** > ( Shader ) )
			);
		break;
		case 'c':
		DX::ThrowIfFailed(
			device->CreateComputeShader( ( *ShaderBlob )->GetBufferPointer( ), ( *ShaderBlob )->GetBufferSize( ), nullptr,
				reinterpret_cast< ID3D11ComputeShader** > ( Shader ) )
		);
		break;
		}

	}

	inline void CreateBuffer( ID3D11Device * device, ID3D11Buffer ** FinalBuffer,
		D3D11_USAGE Usage, D3D11_BIND_FLAG flag, size_t DataSize, int CPUAccessFlag,
		void* data = nullptr, UINT MiscFlag = 0, UINT StructureByteStride = 0 )
	{
		ZeroMemoryAndDeclare( D3D11_BUFFER_DESC, buffDesc );
		ZeroMemoryAndDeclare( D3D11_SUBRESOURCE_DATA, buffData );
		buffDesc.BindFlags = flag;
		buffDesc.ByteWidth = (UINT)DataSize;
		buffDesc.CPUAccessFlags = CPUAccessFlag;
		buffDesc.MiscFlags = MiscFlag;
		buffDesc.StructureByteStride = StructureByteStride;
		buffDesc.Usage = Usage;
		if ( data /*!= nullptr*/ )
		{
			buffData.pSysMem = data;
			DX::ThrowIfFailed(
				device->CreateBuffer( &buffDesc, &buffData, FinalBuffer )
				);
		}
		else
		{
			DX::ThrowIfFailed(
				device->CreateBuffer( &buffDesc, nullptr, FinalBuffer )
				);
		}
	}
}