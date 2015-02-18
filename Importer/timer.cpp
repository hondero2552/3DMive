#include "timer.h"

Timer::Timer(void)
{
    QueryPerformanceFrequency(&m_iProcessorFrequency);
    m_iProcessorFrequency.QuadPart /= 1000;
}

Timer::~Timer(void)
{

}