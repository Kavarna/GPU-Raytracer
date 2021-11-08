#include "commonincludes.h"

namespace DX
{
	MicrosoftPointer( ID3D11RasterizerState ) NoCulling;
	MicrosoftPointer( ID3D11BlendState ) InverseBlending;
	void InitializeStates( ID3D11Device * device )
	{

		ZeroMemoryAndDeclare( D3D11_RASTERIZER_DESC, rastDesc );
		rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		ThrowIfFailed( device->CreateRasterizerState( &rastDesc, &NoCulling ) );
		ZeroMemoryAndDeclare( D3D11_BLEND_DESC, blendDesc );
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[ 0 ].BlendEnable = true;
		blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT;
		blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND::D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND::D3D11_BLEND_DEST_COLOR;
		blendDesc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
		ThrowIfFailed( device->CreateBlendState( &blendDesc, &InverseBlending ) );

	}
}