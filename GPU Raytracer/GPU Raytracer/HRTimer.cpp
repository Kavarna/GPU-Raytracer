#include "HRTimer.h"



CHRTimer::CHRTimer( )
{ }


CHRTimer::~CHRTimer( )
{ }

void CHRTimer::Start( )
{
	QueryPerformanceFrequency( &mFrequency );
	QueryPerformanceCounter( &mStartTime );
	mLastCurrent = mStartTime;
	m_framePerSecond = m_currentFrames;
	m_currentFrames = 0;
	m_totalTime = 0;
}

void CHRTimer::Frame( )
{
	m_currentFrames++;
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter( &currentTime );
	m_frameTime = float( currentTime.QuadPart - mLastCurrent.QuadPart ) / float( mFrequency.QuadPart );
	mLastCurrent = currentTime;
	m_totalTime += m_frameTime;
	m_totalApplicationTime += m_frameTime;
}

int CHRTimer::GetFPS( )
{
	return m_framePerSecond;
}

float CHRTimer::GetFrameTime( )
{
	return m_frameTime;
}

float CHRTimer::GetTimeSinceLastStart( )
{
	return m_totalTime;
}

float CHRTimer::GetTotalTime( )
{
	return m_totalApplicationTime;
}
