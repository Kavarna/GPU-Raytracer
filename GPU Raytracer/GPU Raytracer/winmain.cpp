#include "Scene.h"

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine, int iShow )
{
	srand( ( unsigned int ) time( NULL ) );
	Scene* SC = new Scene( hInstance,
#if DEBUG || _DEBUG
		false
#else
		false
#endif
		);
	SC->Run( );
	delete SC;
	return 0;
}