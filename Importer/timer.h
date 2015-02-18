#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif

#ifndef TIMER_H
#define TIMER_H

#include "Debug_ALL.h"
char mine[ ];
class Timer
{
    LARGE_INTEGER m_iCurrentTick;
    LARGE_INTEGER m_iPreviousTick;
    LARGE_INTEGER m_iProcessorFrequency;
public:
    Timer(void);
    ~Timer(void);
    void tick(void) { QueryPerformanceCounter(&m_iCurrentTick); }
    inline void RestartCount(void) { m_iPreviousTick.QuadPart = m_iCurrentTick.QuadPart;}
    inline double TimeElapsed(void) 
    {  
        return static_cast<double>((m_iCurrentTick.QuadPart - m_iPreviousTick.QuadPart)/(m_iProcessorFrequency.QuadPart));
    }
};
#endif