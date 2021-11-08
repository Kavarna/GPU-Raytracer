#pragma once

#include "commonincludes.h"
#include "Input.h"
#include "HRTimer.h"
#include "Quad.h"
#include "RayTracing.h"
#include "Physics.h"
#include "Font.h"
#include "Text.h"
#include "C2DShader.h"
#include "Square.h"

ALIGN16 class Scene
{
public:
	static constexpr const float MAX_DISTANCE = 1000.0f;
	static constexpr const int MAX_DEPTH = 5;
	static constexpr const float GROUND_RADIUS = 50000.f;
	static constexpr const int RingTesselation = 10;
	static constexpr const float RingRadius = 3.0f;
	static constexpr const float MaxDistanceFromCenterSq = 1600;
	static constexpr const float TimeUntillSphereDelete = 3; // Seconds
	static constexpr const float MinimumCamDistance = 6.0f;
	static constexpr const float CameraDistanceStep = 1.0f;
	static constexpr const char HighscoreFileName[ ] = "Highscore.ab";
private:
	struct SSphere
	{
		RayTracing::SSphere rtSphere;
		btRigidBody * pSphere;
		float fRemainingTime;
		bool bDelete = false;
	};
public:
	enum class EGameState
	{
		Game, Transition, GameOver
	};
	static constexpr const float TransitionTime = 5.0f;
private:
	HWND mWindow;
	HINSTANCE mInstance;
	D3D11_VIEWPORT mFullscreenViewport;

	CHRTimer mTimer;
	WCHAR* mGPUDescription;

	int mWidth;
	int mHeight;

	int mScore;
	int mHighscore;

	std::shared_ptr<CInput> mInput;
	std::shared_ptr<QuadShader> mQuadShader;
	std::shared_ptr<C2DShader> m2DShader;
	
	std::shared_ptr<CFont> m32OpenSans;

	std::unique_ptr<Square> mCrosshair;
	std::unique_ptr<CText> mFPSText;
	std::unique_ptr<CText> mScoreText;
	std::unique_ptr<CText> mHighscoreText;
	std::unique_ptr<CText> mGameOverText;
#if DEBUG || _DEBUG
	std::unique_ptr<CText> mDebugText;
#endif

	std::unique_ptr<Quad> mQuad;

	DirectX::XMMATRIX mOrthoMatrix;
	DirectX::XMFLOAT3 mAmbient;

	RayTracing::SCamera mCamera;
	RayTracing::SOtherInfo mRayInfo;
	
	std::unique_ptr<RayTracing> mRayTracer;

	std::vector<SSphere*> mSpheres;
	std::vector<RayTracing::STriangle> mTriangles;
	DirectX::XMFLOAT3 mRingCenter;
	SSphere * mHitSphere;
	bool bCanShoot = true;

	std::vector<DirectX::XMFLOAT3> mVertices;
	std::vector<DWORD> mIndices;
	btRigidBody* mMirror;

	float mAccumulatedTime;

	EGameState mState;
	DirectX::XMFLOAT4 mNewCamPos;

	float mRotationX;
	float mRotationY;

private: // D3D objects
	Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mImmediateContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mBackbuffer;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
public:
	Scene( HINSTANCE Instance, bool bFullscreen = false );
	~Scene( );
public:
	void Run( );
private:
	void CalculateBillboard( DirectX::XMFLOAT3 const& object, DirectX::XMFLOAT4 const& camPos,
		float& xRotation, float& yRotation );
private:
	void InitWindow( bool bFullscreen );
	void InitD3D( bool bFullscreen );
	void InitShaders( );
	void Init2D( );
	void InitModels( );
	void AddModelsToScene( );
	void InitRing( );
	void SetupLights( );
private:
	void Rotate( float frameTime, float fromX, float toX, float fromY, float toY,
		float& outX, float& outY );
	void Move( float frameTime, DirectX::XMFLOAT4 const& from, DirectX::XMFLOAT4 const& to,
		DirectX::XMFLOAT4& out );
	DirectX::XMFLOAT4 GenerateCameraPosition( );
	void ResetGame( bool updatePosition = true );
	void UpdateGame( float frameTime );
	void UpdateGameOver( float frameTime );
	void UpdateTransition( float frameTime );
private:
	void EnableBackbuffer( );
	void Update( );
	void Render( );
public:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam );
public:
	inline void* operator new( size_t size )
	{
		return _aligned_malloc( size, 16 );
	}
	inline void operator delete( void* object )
	{
		return _aligned_free( object );
	}
};

