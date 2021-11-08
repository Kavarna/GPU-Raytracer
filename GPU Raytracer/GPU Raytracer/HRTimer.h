#pragma once

#include <Windows.h>

class CHRTimer sealed
{
private:
	float m_frameTime;
	float m_totalTime;
	float m_totalApplicationTime;
	int m_currentFrames;
	int m_framePerSecond;

	LARGE_INTEGER mStartTime;
	LARGE_INTEGER mLastCurrent;
	LARGE_INTEGER mFrequency;

public:
	CHRTimer( );
	~CHRTimer( );
	
public:
	void Start( );
	void Frame( );

public:
	int GetFPS( );
	float GetFrameTime( );
	float GetTimeSinceLastStart( );
	float GetTotalTime( );
};

