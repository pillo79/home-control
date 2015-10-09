#include "plc_lib.h"

#include <stdlib.h>
#include <sys/time.h>

uint64_t Timer::m_now = 0;

void Timer::tick()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	m_now = (tv.tv_sec*1000000ULL)+tv.tv_usec;
}

DelayRiseTimer::DelayRiseTimer()
	: Timer()
	, m_lastInput(true)
	, m_lastEdge(m_now)
{

}

bool DelayRiseTimer::update(unsigned delayus, bool input)
{
	if (!input) {
		m_lastInput = false;
		return false;
	} else if (m_lastInput) {
		return (m_now-m_lastEdge > delayus);
	} else {
		m_lastInput = true;
		m_lastEdge = m_now;
		return false;
	}
}

DelayFallTimer::DelayFallTimer()
	: Timer()
	, m_lastInput(false)
	, m_lastEdge(0)
{

}

bool DelayFallTimer::update(unsigned delayus, bool input)
{
	if (input) {
		m_lastInput = true;
		return true;
	} else if (!m_lastInput) {
		return (m_now-m_lastEdge < delayus);
	} else {
		m_lastInput = false;
		m_lastEdge = m_now;
		return true;
	}
}

PeriodicTimer::PeriodicTimer()
	: Timer()
	, m_lastInput(false)
	, m_lastEdge(m_now)
{

}

bool PeriodicTimer::update(unsigned delayus, bool input)
{
	if (!input) {
		m_lastInput = false;
		return false;
	} else if (m_lastInput) {
		if (m_now-m_lastEdge >= delayus) {
			// FIXME: more accurate but does not work for some reason
			// m_lastEdge += delayus;
			m_lastEdge = m_now;
			return true;
		}
		return false;
	} else {
		m_lastInput = true;
		m_lastEdge = m_now;
		return false;
	}
}
