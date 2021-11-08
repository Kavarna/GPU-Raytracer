#include "Input.h"



CInput::CInput( )
{
	ZeroMemory( this, sizeof( CInput ) );
}

bool CInput::Initialize( HINSTANCE hInstance, HWND hWnd )
{
	HRESULT hr;
	hr = DirectInput8Create( hInstance, // Use Direct Input for this instance
		DIRECTINPUT_VERSION, // This version of Direct Input
		IID_IDirectInput8, // Use this instance
		reinterpret_cast<void**>(&m_DirectInput), // And store the result here
		NULL ); // No punkOuter
	if ( FAILED( hr ) )
		return false;

	hr = m_DirectInput->CreateDevice( GUID_SysKeyboard, // Create a keyboard
		&m_Keyboard, // and store it here
		NULL ); // No punkOuter
	if ( FAILED( hr ) )
		return false;

	hr = m_DirectInput->CreateDevice( GUID_SysMouse, // Create a mouse
		&m_Mouse, // and store it here
		NULL ); // No punkOuter
	if ( FAILED( hr ) )
		return false;
	
	int bExclusive;
#if DEBUG || _DEBUG
	bExclusive = DISCL_EXCLUSIVE;
#else
	bExclusive = DISCL_EXCLUSIVE;
#endif
	hr = m_Mouse->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | bExclusive );
	if ( FAILED( hr ) )
		return false;
	hr = m_Mouse->SetDataFormat( &c_dfDIMouse );
	if ( FAILED( hr ) )
		return false;

	hr = m_Keyboard->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | DISCL_NOWINKEY | bExclusive );
	if ( FAILED( hr ) )
		return false;
	hr = m_Keyboard->SetDataFormat( &c_dfDIKeyboard );
	if ( FAILED( hr ) )
		return false;

	return true;

}

void CInput::Frame( )
{
	m_lastMouseState = m_currentMouseState;
	for ( UINT i = 0; i < m_vecSpecialKeys.size( ); ++i )
		m_lastKeyboard[ m_vecSpecialKeys[ i ] ] = m_currentKeyboard[ m_vecSpecialKeys[ i ] ];
	m_Keyboard->Acquire( );
	m_Mouse->Acquire( );
	m_Keyboard->GetDeviceState( sizeof( m_currentKeyboard ), m_currentKeyboard );
	m_Mouse->GetDeviceState( sizeof( m_currentMouseState ), &m_currentMouseState );
}

bool CInput::isKeyPressed( BYTE key )
{
	return ( m_currentKeyboard[ key ] & 0x80 ) > 0;
}

bool CInput::isSpecialKeyPressed( BYTE key )
{
	return m_currentKeyboard[ key ] & 0x80 && // if the key was pressed this frame
		!(m_lastKeyboard[ key ] & 0x80); // and it was not pressed in the previous frame
}

bool CInput::addSpecialKey( BYTE key )
{
	for ( UINT i = 0; i < m_vecSpecialKeys.size( ); ++i ) // Try to find the key
		if ( m_vecSpecialKeys[ i ] == key ) // if we found the key
			return false; // we didn't add anything so we return false
	m_vecSpecialKeys.push_back( key ); // if we didn't find the key, we just push it back into the vector
	return true; // and return
}

bool CInput::removeSpecialKey( BYTE key )
{
	bool bFound = false; // this variable will be used to see if we found any key to delete
	for ( UINT i = 0; i < m_vecSpecialKeys.size( ); ++i ) // search through the vector with special keys
		if ( m_vecSpecialKeys[ i ] == key ) // if we found the key
		{
			m_vecSpecialKeys.erase( m_vecSpecialKeys.begin( ) + i ); // delete it
			bFound = true; // mark that we found it and deleted it
		}
	return bFound; // return the result
}

void CInput::Shutdown( )
{
	SAFE_RELEASE( m_Mouse );
	SAFE_RELEASE( m_Keyboard );
	SAFE_RELEASE( m_DirectInput );
}

CInput::~CInput( )
{
	Shutdown( );
}
