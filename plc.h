#ifndef __PLC_H__
#define __PLC_H__

#include <stdint.h>

#define DELAY_MSEC(x) ((x)*1000)
#define DELAY_SEC(x) DELAY_MSEC((x)*1000)
#define DELAY_MIN(x) DELAY_SEC((x)*60)

class Timer {
	public:
		Timer() { }
		virtual ~Timer() { }
		virtual bool update(unsigned usdelay, bool input) { return input; }
	public:
		static void tick();
	protected:
		static uint64_t m_now;
};

class DelayRiseTimer : public Timer {
	public:
		DelayRiseTimer();
		virtual ~DelayRiseTimer() { }
		virtual bool update(unsigned usdelay, bool input);
	private:
		bool m_lastInput;
		uint64_t m_lastEdge;
};

class DelayFallTimer : public Timer {
	public:
		DelayFallTimer();
		virtual ~DelayFallTimer() { }
		virtual bool update(unsigned usdelay, bool input);
	private:
		bool m_lastInput;
		uint64_t m_lastEdge;
};

class PeriodicTimer : public Timer {
	public:
		PeriodicTimer();
		virtual ~PeriodicTimer() { }
		virtual bool update(unsigned usdelay, bool input);
	private:
		bool m_lastInput;
		uint64_t m_lastEdge;
};

#endif /* __PLC_H__ */
