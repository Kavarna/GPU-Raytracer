#include "Scene.h"



Scene::Scene( HINSTANCE Instance, bool bFullscreen ) :
	mInstance( Instance ),
	mScore( 0 ),
	mState( Scene::EGameState::Game )
{
	try
	{
		Physics::CreateInstance( );
		InitWindow( bFullscreen );
		InitD3D( bFullscreen );
		InitShaders( );
		Init2D( );
		InitModels( );
		AddModelsToScene( );
		SetupLights( );
		std::ifstream ifScore( HighscoreFileName );
		if ( ifScore.is_open( ) )
		{
			ifScore >> mHighscore;
			ifScore.close( );
		}
		else
		{
			mHighscore = 0;
		}
		ResetGame( );
	}
	CATCH;
}


Scene::~Scene( )
{
	for ( unsigned int i = 0; i < mSpheres.size( ); ++i )
	{
		Physics::GetInstance( )->DeleteRigidBody( mSpheres[ i ]->pSphere );
		delete mSpheres[ i ]->pSphere->getCollisionShape( );
		delete mSpheres[ i ]->pSphere->getMotionState( );
		delete mSpheres[ i ]->pSphere;
	}
	Physics::DestroyInstance( );

	mDevice.Reset( );
	mImmediateContext.Reset( );
	mBackbuffer.Reset( );
	mSwapChain.Reset( );

	mInput.reset( );

	UnregisterClass( ENGINE_NAME, mInstance );
	DestroyWindow( mWindow );

	std::ofstream ofPrint( HighscoreFileName );
	ofPrint << mHighscore;
}


void Scene::InitWindow( bool bFullscreen )
{
	ZeroMemoryAndDeclare( WNDCLASSEX, wndClass );
	wndClass.cbSize = sizeof( WNDCLASSEX );
	wndClass.hbrBackground = ( HBRUSH ) ( GetStockObject( DKGRAY_BRUSH ) );
	wndClass.hInstance = mInstance;
	wndClass.lpfnWndProc = Scene::WndProc;
	wndClass.lpszClassName = ENGINE_NAME;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	if ( !RegisterClassEx( &wndClass ) )
		throw std::exception( "Couldn't register window class" );
	
	if ( !bFullscreen )
	{
		mWidth = 800;
		mHeight = 600;
	}
	else
	{
		mWidth = GetSystemMetrics( SM_CXSCREEN );
		mHeight = GetSystemMetrics( SM_CYSCREEN );
	}

	mWindow = CreateWindow( ENGINE_NAME, ENGINE_NAME,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, mWidth, mHeight,
		nullptr, nullptr, mInstance, nullptr );
	if ( !mWidth )
		throw std::exception( "Couldn't create window" );

	UpdateWindow( mWindow );
	ShowWindow( mWindow, SW_SHOWNORMAL );
	SetFocus( mWindow );

	mFullscreenViewport.Width = ( FLOAT ) mWidth;
	mFullscreenViewport.Height = ( FLOAT ) mHeight;
	mFullscreenViewport.TopLeftX = 0;
	mFullscreenViewport.TopLeftY = 0;
	mFullscreenViewport.MinDepth = 0.0f;
	mFullscreenViewport.MaxDepth = 1.0f;
	mOrthoMatrix = DirectX::XMMatrixOrthographicLH( ( float ) mWidth, ( float ) mHeight, DX::CamNear, DX::CamFar );
}

void Scene::InitD3D( bool bFullscreen )
{
	IDXGIFactory * Factory;
	DX::ThrowIfFailed( CreateDXGIFactory( __uuidof( IDXGIFactory ),
		reinterpret_cast< void** >( &Factory ) ) );
	IDXGIAdapter * Adapter;
	DX::ThrowIfFailed( Factory->EnumAdapters( 0, &Adapter ) );
	IDXGIOutput * Output;
	DX::ThrowIfFailed( Adapter->EnumOutputs( 0, &Output ) );
	UINT NumModes;
	DX::ThrowIfFailed( Output->GetDisplayModeList( DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &NumModes, nullptr ) );
	DXGI_MODE_DESC * Modes = new DXGI_MODE_DESC[ NumModes ];
	DX::ThrowIfFailed( Output->GetDisplayModeList( DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &NumModes, Modes ) );
	DXGI_MODE_DESC FinalMode;
	bool bFound = false;
	for ( size_t i = 0; i < NumModes; ++i )
	{
		if ( Modes[ i ].Width == mWidth && Modes[ i ].Height == mHeight )
		{
			FinalMode = DXGI_MODE_DESC( Modes[ i ] );
			bFound = true;
			break;
		}
	}
	if ( !bFound )
	{
		FinalMode.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		FinalMode.Width = mWidth;
		FinalMode.Height = mHeight;
		FinalMode.RefreshRate.Denominator = 0;
		FinalMode.RefreshRate.Numerator = 60;
		FinalMode.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
		FinalMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	}
	delete[ ] Modes;
	DXGI_ADAPTER_DESC GPU;
	Adapter->GetDesc( &GPU );
	mGPUDescription = GPU.Description;
	ZeroMemoryAndDeclare( DXGI_SWAP_CHAIN_DESC, swapDesc );
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc = FinalMode;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.OutputWindow = mWindow;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = !bFullscreen;

	// MSAA
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT flags = 0;
#if DEBUG || _DEBUG
	flags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driver =
#if defined NO_GPU
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_WARP
#else
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE
#endif
		;

	DX::ThrowIfFailed(
		D3D11CreateDeviceAndSwapChain( NULL, driver, NULL, flags,
			NULL, NULL, D3D11_SDK_VERSION, &swapDesc, &mSwapChain, &mDevice, NULL, &mImmediateContext )
	);

	ID3D11Texture2D * backBufferResource;
	DX::ThrowIfFailed( mSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ),
		reinterpret_cast< void** >( &backBufferResource ) ) );
	DX::ThrowIfFailed(
		mDevice->CreateRenderTargetView(
			backBufferResource, nullptr, &mBackbuffer
		)
	);

	backBufferResource->Release( );
	Factory->Release( );
	Adapter->Release( );
	Output->Release( );
	mInput = std::make_shared<CInput>( );
	mInput->Initialize( mInstance, mWindow );
	DX::InitializeStates( mDevice.Get( ) );
	mImmediateContext->RSSetState( DX::NoCulling.Get( ) );
	mImmediateContext->RSSetViewports( 1, &mFullscreenViewport );
}

void Scene::InitShaders( )
{
	mQuadShader = std::make_shared<QuadShader>( mDevice, mImmediateContext );
	m2DShader = std::make_shared<C2DShader>( mDevice.Get( ), mImmediateContext.Get( ) );
}

void Scene::Init2D( )
{
	mCrosshair = std::make_unique<Square>( mDevice.Get( ), mImmediateContext.Get( ),
		m2DShader, mWidth, mHeight, 16, 16, ( LPWSTR ) L"Data/Crosshair.png" );
	mCrosshair->TranslateTo( float( mWidth ) / 2, float( mHeight ) / 2 );

	m32OpenSans = std::make_shared<CFont>( mDevice.Get( ), mImmediateContext.Get( ),
		( LPWSTR ) L"Data/32OpenSans.fnt" );
	
	mFPSText = std::make_unique<CText>( mDevice.Get( ), mImmediateContext.Get( ),
		m2DShader, m32OpenSans, mWidth, mHeight );
	mScoreText = std::make_unique<CText>( mDevice.Get( ), mImmediateContext.Get( ),
		m2DShader, m32OpenSans, mWidth, mHeight );
	mHighscoreText = std::make_unique<CText>( mDevice.Get( ), mImmediateContext.Get( ),
		m2DShader, m32OpenSans, mWidth, mHeight );
	mGameOverText = std::make_unique<CText>( mDevice.Get( ), mImmediateContext.Get( ),
		m2DShader, m32OpenSans, mWidth, mHeight );
#if DEBUG || _DEBUG
	mDebugText = std::make_unique<CText>( mDevice.Get( ), mImmediateContext.Get( ),
		m2DShader, m32OpenSans, mWidth, mHeight );
#endif
}

void Scene::InitModels( )
{
	mRayTracer = std::make_unique<RayTracing>( mDevice, mImmediateContext, mWidth, mHeight );
	mRayInfo.Height = mHeight;
	mRayInfo.Width = mWidth;
	mRayInfo.MaxRecursion = MAX_DEPTH;
	mRayInfo.RayMaxLength = DX::CamFar;
	mRayTracer->UpdateOtherInfo( mRayInfo );
	mQuad = std::make_unique<Quad>( mDevice, mImmediateContext, mQuadShader, mWidth, mHeight );
}

void Scene::AddModelsToScene( )
{
	auto Models = mRayTracer->GetModelsRW( );
	auto AddModel = [&] ( DirectX::XMFLOAT3 Position, float Radius, DirectX::XMFLOAT4 Color, float Reflectivity, float mass = 1.0f )
	{
		Models->Spheres[ Models->NumSpheres ].Color = Color;
		Models->Spheres[ Models->NumSpheres ].Pos = Position;
		Models->Spheres[ Models->NumSpheres ].Radius = Radius;
		Models->Spheres[ Models->NumSpheres ].Reflectivity = Reflectivity;
		auto pSphere = Physics::GetInstance( )->CreateSphere( mass, Radius,
			btVector3( Position.x, Position.y, Position.z ), btVector3( 0, 0, 0 ), mass > 0 );
		Scene::SSphere * s = new Scene::SSphere( );
		s->pSphere = pSphere;
		s->rtSphere = Models->Spheres[ Models->NumSpheres ];
		mSpheres.push_back( s );
		Models->NumSpheres++;
	};
	auto AddTriangle = [&] ( DirectX::XMFLOAT3 A, DirectX::XMFLOAT3 B, DirectX::XMFLOAT3 C,
		DirectX::XMFLOAT4 Color, float Reflectivity )
	{
		int lastIndex = 0;
		if ( mIndices.size( ) )
		{
			lastIndex = mIndices[ mIndices.size( ) - 1 ] + 1;
		}
		for ( int i = 0; i < 3; ++i )
			mIndices.push_back( lastIndex + i );
		mVertices.push_back( A );
		mVertices.push_back( B );
		mVertices.push_back( C );
		Models->Triangles[ Models->NumTriangles ].A = A;
		Models->Triangles[ Models->NumTriangles ].B = B;
		Models->Triangles[ Models->NumTriangles ].C = C;
		Models->Triangles[ Models->NumTriangles ].Color = Color;
		Models->Triangles[ Models->NumTriangles ].Reflectivity = Reflectivity;
		mTriangles.push_back( Models->Triangles[ Models->NumTriangles ] );
		Models->NumTriangles++;
	};
	AddModel( DirectX::XMFLOAT3( 0.0f, -GROUND_RADIUS, 0.0f ), GROUND_RADIUS, DirectX::XMFLOAT4( 0.48f, 0.98f, 0.0f, 1.0f ), 0.05f,0.0f );
	//AddModel( DirectX::XMFLOAT3( 0.0f, 1.0f, 5.0f ), 1, DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), 0.3f );
	//AddModel( DirectX::XMFLOAT3( 2.0f, 1.0f, 5.0f ), 1, DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ), 0.3f );
	//AddModel( DirectX::XMFLOAT3( 1.0f, 2.7f, 5.0f ), 1, DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), 0.3f );
	AddTriangle( DirectX::XMFLOAT3( -2.5f, 5.0f, 0.0f ), DirectX::XMFLOAT3( -2.5f, 0.0f, 0.0f ), DirectX::XMFLOAT3( 2.5f, 0.0f, 0.0f ),
		DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), 1.0f );
	AddTriangle( DirectX::XMFLOAT3( 2.5f, 5.0f, 0.0f ), DirectX::XMFLOAT3( -2.5f, 5.0f, 0.0f ), DirectX::XMFLOAT3( 2.5f, 0.0f, 0.0f ),
		DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), 1.0f );

	mMirror = Physics::GetInstance( )->CreateCustomStaticRigidBody( mVertices, mIndices, btVector3( 0, 0, 0 ), btVector3( 0, 0, 0 ) );

	mCamera.CamPos = DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f );

	InitRing( );
}

void Scene::InitRing( )
{
	auto Models = mRayTracer->GetModelsRW( );
	auto AddModel = [&] ( DirectX::XMFLOAT3 Position, float Radius, DirectX::XMFLOAT4 Color, float Reflectivity, float mass = 1.0f )
	{
		Models->Spheres[ Models->NumSpheres ].Color = Color;
		Models->Spheres[ Models->NumSpheres ].Pos = Position;
		Models->Spheres[ Models->NumSpheres ].Radius = Radius;
		Models->Spheres[ Models->NumSpheres ].Reflectivity = Reflectivity;
		auto pSphere = Physics::GetInstance( )->CreateSphere( mass, Radius,
			btVector3( Position.x, Position.y, Position.z ), btVector3( 0, 0, 0 ), mass > 0 );
		Scene::SSphere * s = new Scene::SSphere( );
		s->pSphere = pSphere;
		s->rtSphere = Models->Spheres[ Models->NumSpheres ];
		mSpheres.push_back( s );
		Models->NumSpheres++;
	};
	float thetaStep = DirectX::XM_2PI / RingTesselation;
	float addToX = 0.0f;
	float addToY = 1.0f;
	float addToZ = 4.0f;
	mRingCenter = DirectX::XMFLOAT3( addToX, addToY, addToZ );

	for ( int i = 0; i < RingTesselation; ++i )
	{
		float x = cos( i * thetaStep ) * RingRadius;
		float y = 0.0f;
		float z = sin( i *thetaStep ) * RingRadius;
		x += addToX;
		y += addToY;
		z += addToZ;
	
		AddModel( DirectX::XMFLOAT3( x, y, z ), 1.0f, DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), 1.0f, 0.0f );
	}

}

void Scene::SetupLights( )
{
	RayTracing::SLights lights;
	lights.Sun.Direction = DirectX::XMFLOAT3( 0.3f, -1.0f, 0.0f );
	lights.Sun.Diffuse = DirectX::XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	lights.Sun.Ambient = DirectX::XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	lights.NumPointLights = 1;
	lights.PointLights[ 0 ].Diffuse = DirectX::XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
	lights.PointLights[ 0 ].Position = DirectX::XMFLOAT3( 0.0f, 10.0f, -4.0f );
	mRayTracer->UpdateLights( lights );
}

LRESULT Scene::WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	switch ( Message )
	{
	case WM_QUIT:
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		break;
	}
	return DefWindowProc( hWnd, Message, wParam, lParam );
}

void Scene::CalculateBillboard( DirectX::XMFLOAT3 const& object, DirectX::XMFLOAT4 const& camPos,
	float& xRotation, float& yRotation )
{
	yRotation = atan2( object.x - camPos.x, object.z - camPos.z );
	xRotation = -atan2( object.y - camPos.y, camPos.y );
}

DirectX::XMFLOAT4 Scene::GenerateCameraPosition( )
{
	using namespace DirectX;
	XMFLOAT4 ret;
	float x, y, z;
	x = float( rand( ) ) / RAND_MAX;
	x = x * 2.0f - 1.0f;
	y = float( rand( ) ) / RAND_MAX;
	z = 1.0f + float( rand( ) ) / RAND_MAX;

	XMVECTOR Direction = XMVectorSet( x, y, z, 0.0f );
	Direction = XMVector3Normalize( Direction );

	Direction *= MinimumCamDistance + mScore * CameraDistanceStep;
	if ( XMVectorGetY( Direction ) < mScore + 2.0f )
		Direction = XMVectorSetY( Direction, mScore + 2.0f );

	XMVECTOR RingPos;
	RingPos = XMLoadFloat3( &mRingCenter );

	XMStoreFloat4( &ret, RingPos + Direction );
	return ret;
}

void Scene::ResetGame( bool bUseNewCamPos )
{
	mScore = 0;
	if ( bUseNewCamPos )
	{
		DirectX::XMFLOAT4 newCamPos = GenerateCameraPosition( );
		mCamera.CamPos.x = newCamPos.x;
		mCamera.CamPos.y = newCamPos.y;
		mCamera.CamPos.z = newCamPos.z;
	}
	CalculateBillboard( mRingCenter, mCamera.CamPos, mRotationX, mRotationY );
}

void Scene::Run( )
{
	MSG Message;
	mTimer.Start( );
	while ( true )
	{
		if ( PeekMessage( &Message, nullptr, 0, 0, PM_REMOVE ) )
		{
			if ( Message.message == WM_QUIT )
				break;
			TranslateMessage( &Message );
			DispatchMessage( &Message );
		}
		else
		{
			if ( mInput->isKeyPressed( DIK_ESCAPE ) )
				break;
			if ( mTimer.GetTimeSinceLastStart( ) > 1.0f )
				mTimer.Start( );
			Update( );
			Render( );
		}
	}
}

void Scene::EnableBackbuffer( )
{
	mImmediateContext->RSSetViewports( 1, &mFullscreenViewport );
	mImmediateContext->OMSetRenderTargets( 1, mBackbuffer.GetAddressOf( ), nullptr );
}

void Scene::Update( )
{
	mTimer.Frame( );
	mInput->Frame( );
	float frameTime = mTimer.GetFrameTime( );

#if DEBUG || _DEBUG
	frameTime = 1.f / 60.f;
#endif

	Physics::GetInstance( )->Update( frameTime );

#if DEBUG || _DEBUG
	wchar_t buffer[ 500 ];
	swprintf_s( buffer, L"%ws: %d, %.2f", ENGINE_NAME, mTimer.GetFPS( ), frameTime );
	SetWindowText( mWindow, buffer );
#endif
	switch ( mState )
	{
	case Scene::EGameState::Game:
		UpdateGame( frameTime );
		break;
	case Scene::EGameState::Transition:
		UpdateTransition( frameTime );
		break;
	case Scene::EGameState::GameOver:
		UpdateGameOver( frameTime );
		break;
	default:
		break;
	}
	
}

void Scene::UpdateGame( float frameTime )
{
	mRotationY += mInput->GetHorizontalMouseMove( ) * 0.001f;
	mRotationX += mInput->GetVerticalMouseMove( ) * 0.001f;
	DX::clamp( mRotationY, -1.f, 1.f );
	DX::clamp( mRotationX, -1.f, 1.f );

	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationX( mRotationX ) * DirectX::XMMatrixRotationY( mRotationY );
	DirectX::XMVECTOR Forward, Right;
	Forward = DirectX::XMVector3TransformCoord(
		DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ),
		Rotation );
	Right = DirectX::XMVector3TransformCoord(
		DirectX::XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ),
		Rotation );
	if ( mInput->isKeyPressed( DIK_LSHIFT ) )
		frameTime *= 10;
	DirectX::XMVECTOR Multiplier = DirectX::XMVectorSet( frameTime, frameTime, frameTime, frameTime );
	Right = DirectX::XMVectorMultiply( Right, Multiplier );
	Forward = DirectX::XMVectorMultiply( Forward, Multiplier );

#if DEBUG || _DEBUG
	if ( mInput->isKeyPressed( DIK_P ) )
	{
		if ( mInput->isKeyPressed( DIK_W ) )
		{
			DirectX::XMVECTOR CamPos;
			CamPos = DirectX::XMLoadFloat4( &mCamera.CamPos );
			CamPos = DirectX::XMVectorAdd( CamPos, Forward );
			DirectX::XMStoreFloat4( &mCamera.CamPos, CamPos );
		}
		if ( mInput->isKeyPressed( DIK_S ) )
		{
			DirectX::XMVECTOR CamPos;
			CamPos = DirectX::XMLoadFloat4( &mCamera.CamPos );
			CamPos = DirectX::XMVectorSubtract( CamPos, Forward );
			DirectX::XMStoreFloat4( &mCamera.CamPos, CamPos );
		}
		if ( mInput->isKeyPressed( DIK_A ) )
		{
			DirectX::XMVECTOR CamPos;
			CamPos = DirectX::XMLoadFloat4( &mCamera.CamPos );
			CamPos = DirectX::XMVectorSubtract( CamPos, Right );
			DirectX::XMStoreFloat4( &mCamera.CamPos, CamPos );
		}
		if ( mInput->isKeyPressed( DIK_D ) )
		{
			DirectX::XMVECTOR CamPos;
			CamPos = DirectX::XMLoadFloat4( &mCamera.CamPos );
			CamPos = DirectX::XMVectorAdd( CamPos, Right );
			DirectX::XMStoreFloat4( &mCamera.CamPos, CamPos );
		}
	}
#endif

	if ( mInput->isSpecialLeftKeyPressed( ) && bCanShoot )
	{
		if ( mSpheres.size( ) < RayTracing::MaxSpheres - 1 )
		{
			auto Models = mRayTracer->GetModelsRW( );
			SSphere * s = new SSphere( );
			float sphereRadius = 0.6f;
			s->fRemainingTime = TimeUntillSphereDelete;
			s->bDelete = true;
			s->rtSphere.Color = DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
			s->rtSphere.Radius = sphereRadius;
			s->rtSphere.Reflectivity = 1.0f;
			s->rtSphere.Pos = DirectX::XMFLOAT3( mCamera.CamPos.x, mCamera.CamPos.y, mCamera.CamPos.z );
			Models->Spheres[ Models->NumSpheres++ ] = s->rtSphere;
			s->pSphere = Physics::GetInstance( )->CreateSphere( 1.0f, sphereRadius,
				btVector3( mCamera.CamPos.x, mCamera.CamPos.y, mCamera.CamPos.z ),
				btVector3( 0, 0, 0 ), true );
			DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationX( mRotationX ) * DirectX::XMMatrixRotationY( mRotationY );

			DirectX::XMVECTOR ViewDirection = DirectX::XMVector4Transform( DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ), Rotation );

			btVector3 direction(
				DirectX::XMVectorGetX( ViewDirection ),
				DirectX::XMVectorGetY( ViewDirection ),
				DirectX::XMVectorGetZ( ViewDirection )
			);
			direction *= 20.0f;
			s->pSphere->setLinearVelocity( direction );
			mSpheres.push_back( s );
			mHitSphere = s;
			bCanShoot = false;
		}
	}

	if ( mHitSphere )
	{
		auto spherePos = mHitSphere->rtSphere.Pos;
		float distance = spherePos.x * spherePos.x + spherePos.y * spherePos.y + spherePos.z * spherePos.z;
		if ( mHitSphere->pSphere->getLinearVelocity( ).length2( ) >= 1.0f )
			mHitSphere->fRemainingTime = TimeUntillSphereDelete;
		else
			mHitSphere->fRemainingTime -= frameTime;
		if ( distance > MaxDistanceFromCenterSq || mHitSphere->fRemainingTime <= 0.0f
#if DEBUG || _DEBUG
			|| mInput->isKeyPressed( DIK_H )
#endif
			)
		{
			float distanceToCenterSq = pow( ( mRingCenter.x - spherePos.x ), 2 ) +
				pow( ( mRingCenter.y - spherePos.y ), 2 ) +
				pow( ( mRingCenter.z - spherePos.z ), 2 );
			float distance = sqrt( distanceToCenterSq );
			if ( distanceToCenterSq <= ( RingRadius ) * ( RingRadius ) )
			{
				mAccumulatedTime = 0;
				mState = EGameState::Transition;
				mScore++;
			}
			else
			{
				mAccumulatedTime = 0;
				mState = EGameState::GameOver;
				mHighscore = mHighscore > mScore ? mHighscore : mScore;
				mScore = 0;
			}
			Physics::GetInstance( )->DeleteRigidBody( mHitSphere->pSphere );
			delete mHitSphere->pSphere->getMotionState( );
			delete mHitSphere->pSphere->getCollisionShape( );
			delete mHitSphere->pSphere;
			delete mHitSphere;
			mSpheres.erase(
				std::remove_if(
					mSpheres.begin( ), mSpheres.end( ),
					[=] ( SSphere * s )
			{
				return s == mHitSphere;
			} ), mSpheres.end( )
				);
			mHitSphere = nullptr;
			bCanShoot = true;
			mNewCamPos = GenerateCameraPosition( );
		}
	}

	auto Spheres = mRayTracer->GetModelsRW( );
	Spheres->NumSpheres = mSpheres.size( );
	mSpheres.erase(
		std::remove_if(
			mSpheres.begin( ), mSpheres.end( ),
			[=] ( SSphere * s )
	{
		auto pObject = s->pSphere;
		btTransform tr = pObject->getWorldTransform( );
		btVector3 origin = tr.getOrigin( );
		s->rtSphere.Pos = DirectX::XMFLOAT3( origin.x( ), origin.y( ), origin.z( ) );
		if ( s->bDelete )
		{
			s->fRemainingTime -= frameTime;
			if ( s->fRemainingTime < 0 )
				return true;
		}
		return false;
	} ), mSpheres.end( )
		);
	for ( unsigned int i = 0; i < mSpheres.size( ); ++i )
	{
		Spheres->Spheres[ i ].Pos = mSpheres[ i ]->rtSphere.Pos;
	}
}

void Scene::Rotate( float frameTime, float fromX, float toX, float fromY, float toY,
	float& outX, float& outY )
{
	float closest = fabs( fromY - ( toY - DirectX::XM_2PI ) );
	int multiplier = -1;
	for ( int i = 0; i <= 1; ++i )
	{
		float len = fabs( fromY - ( toY + i * DirectX::XM_2PI ) );
		if ( len < closest )
		{
			closest = len;
			multiplier = i;
		}
	}
	outY += ( ( toY + multiplier * DirectX::XM_2PI ) - fromY ) * frameTime;
	outX += ( toX - fromX ) * frameTime;
}

void Scene::Move( float frameTime, DirectX::XMFLOAT4 const& from, DirectX::XMFLOAT4 const& to,
	DirectX::XMFLOAT4& out )
{
	out.x += ( to.x - from.x ) * frameTime;
	out.y += ( to.y - from.y ) * frameTime;
	out.z += ( to.z - from.z ) * frameTime;
}

void Scene::UpdateGameOver( float frameTime )
{
	mAccumulatedTime += frameTime;
	float rotX, rotY;
	CalculateBillboard( mRingCenter, mNewCamPos, rotX, rotY );
	Rotate( frameTime, mRotationX, rotX, mRotationY, rotY, mRotationX, mRotationY );
	Move( frameTime, mCamera.CamPos, mNewCamPos, mCamera.CamPos );

	if ( mAccumulatedTime >= TransitionTime )
	{
		
		ResetGame( false );
		mAccumulatedTime = 0;
		mState = EGameState::Game;
	}
}

void Scene::UpdateTransition( float frameTime )
{
	mAccumulatedTime += frameTime;
	float rotX, rotY;
	CalculateBillboard( mRingCenter, mNewCamPos, rotX, rotY );
	Rotate( frameTime, mRotationX, rotX, mRotationY, rotY, mRotationX, mRotationY );
	Move( frameTime, mCamera.CamPos, mNewCamPos, mCamera.CamPos );

	if ( mAccumulatedTime >= TransitionTime )
	{
		mCamera.CamPos = mNewCamPos;
		mAccumulatedTime = 0;
		mState = EGameState::Game;
	}
}

void Scene::Render( )
{
	using namespace DirectX;
	static FLOAT BackColor[ 4 ] = { 0,0,0,0 };
	//EnableBackbuffer( );
	//mImmediateContext->ClearRenderTargetView( mBackbuffer.Get( ), BackColor ); - Useless
	static float screenDistance = 1.0f;
	
	XMMATRIX Rotation = XMMatrixRotationX( mRotationX ) * XMMatrixRotationY( mRotationY );

	XMVECTOR ViewDirection = XMVector4Transform( XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ), Rotation );
	XMVECTOR ScreenCenter = screenDistance * ViewDirection;
	XMVECTOR P0 = ScreenCenter + XMVector4Transform( XMVectorSet( -1.0f, 1.0f, 0.0f, 0.0f ), Rotation );
	XMVECTOR P1 = ScreenCenter + XMVector4Transform( XMVectorSet( 1.0f, 1.0f, 0.0f, 0.0f ), Rotation );
	XMVECTOR P2 = ScreenCenter + XMVector4Transform( XMVectorSet( -1.0f, -1.0f, 0.0f, 0.0f ), Rotation );

	XMStoreFloat4( &mCamera.TopLeftCorner, P0 );
	XMStoreFloat4( &mCamera.TopRightCorner, P1 );
	XMStoreFloat4( &mCamera.BottomLeftCorner, P2 );

	mRayTracer->Trace( mCamera );

	ID3D11RenderTargetView * RTVs = mRayTracer->GetTextureAsRTV( );
	mImmediateContext->OMSetRenderTargets( 1, &RTVs, nullptr );

	char buffer[ 250 ];
	sprintf_s( buffer, "FPS: %d", mTimer.GetFPS( ) );
	mFPSText->Render( mOrthoMatrix, buffer, 0, 0,
		DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) );

	sprintf_s( buffer, "Longest streak: %d", mHighscore );
	float width = mHighscoreText->GetFont( )->EstimateWidth( buffer );
	mHighscoreText->Render( mOrthoMatrix, buffer, mWidth - width, 0,
		DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) );

	sprintf_s( buffer, "Streak: %d", mScore );
	width = mScoreText->GetFont( )->EstimateWidth( buffer );
	int y = mScoreText->GetFont( )->GetHeight( );
	mScoreText->Render( mOrthoMatrix, buffer, ( float ) mWidth - width, ( float ) y,
		DirectX::XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) );

	static FLOAT White[ 4 ] = { 1.f,1.f,1.f,1.f };
	mImmediateContext->OMSetBlendState( DX::InverseBlending.Get( ), White, 0xffffffff );
	mCrosshair->Render( mOrthoMatrix );
	mImmediateContext->OMSetBlendState( nullptr, nullptr, 0xffffffff );

	if ( mState == EGameState::GameOver )
	{
		char buffer[ 40 ];
		sprintf_s( buffer, "Starting over..." );
		float width = mGameOverText->GetFont( )->EstimateWidth( buffer );
		float height = ( float ) mGameOverText->GetFont( )->GetHeight( );
		float x = float( mWidth ) / 2 - width / 2;
		float y = float( mHeight ) / 2 - height / 2;
		mGameOverText->Render( mOrthoMatrix, buffer, x, y,
			DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	}

#if DEBUG || _DEBUG
	sprintf_s( buffer, "DEBUG MODE" );
	mDebugText->Render( mOrthoMatrix, buffer, 0, 35,
		DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) );
#endif

	EnableBackbuffer( );
	mQuad->Render( mOrthoMatrix, mRayTracer->GetTextureAsSRV( ) );

	mSwapChain->Present( 1, 0 );
}