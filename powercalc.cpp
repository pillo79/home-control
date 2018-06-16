#include "powercalc.h"

#include <stdio.h>
#include <string.h>

#define WRAP(x) ((x+SAMPLES_PER_HOUR) % SAMPLES_PER_HOUR)

PowerCalc::PowerCalc(int wh_per_tick)
	: m_wh_per_tick(wh_per_tick)
	, m_samples(new int[SAMPLES_PER_HOUR])
	, m_initLast(true)
{
	bzero(m_samples, SAMPLES_PER_HOUR*sizeof(int));
	bzero(m_last_5W, sizeof(m_last_5W));
	bzero(m_last_25W, sizeof(m_last_25W));
	bzero(m_last_100W, sizeof(m_last_100W));
}

PowerCalc::~PowerCalc()
{
	delete m_samples;
}

void PowerCalc::restart()
{
	m_initLast = true;
}

void PowerCalc::addSample(int now)
{
	if (m_initLast) {
		m_last = now;
		m_lastDelta = now;
		m_initLast = false;
		return;
	}

	// truncate to 16 bits and wrap
	int wrappedValue = ((1<<16)-m_last+now) & ((1<<16)-1);

	// update running total
	m_runningPower_5W += wrappedValue-m_samples[WRAP(m_sampleIdx-SAMPLES_PER_HOUR/5)];
	m_runningPower_25W += wrappedValue-m_samples[WRAP(m_sampleIdx-SAMPLES_PER_HOUR/25)];
	m_runningPower_100W += wrappedValue-m_samples[WRAP(m_sampleIdx-SAMPLES_PER_HOUR/100)];
	m_runningEnergy += wrappedValue;

	// update moving averages
	m_maPower_5W += m_runningPower_5W-m_last_5W[m_sampleIdx & 7];
	m_maPower_25W += m_runningPower_25W-m_last_25W[m_sampleIdx & 7];
	m_maPower_100W += m_runningPower_100W-m_last_100W[m_sampleIdx & 7];

//	printf("%3i now %04x last %04x wrap %04x totals %5i %04x %5i %04x %5i %04x %04x\n", m_sampleIdx, now, m_last, wrappedValue, m_runningPower_5W*5, m_runningPower_5W, m_runningPower_25W*25, m_runningPower_25W, m_runningPower_100W*100, m_runningPower_100W, m_runningEnergy);
	printf("%4i +%-2i %4i %4i %4i %5i | ", m_sampleIdx, wrappedValue, m_runningPower_5W*5, m_runningPower_25W*25, m_runningPower_100W*100, m_runningEnergy);

	m_samples[m_sampleIdx] = wrappedValue;
	m_last_5W[m_sampleIdx & 7] = m_runningPower_5W;
	m_last_25W[m_sampleIdx & 7] = m_runningPower_25W;
	m_last_100W[m_sampleIdx & 7] = m_runningPower_100W;
	m_sampleIdx = WRAP(m_sampleIdx+1);
	m_last = now;
}

int PowerCalc::getCurrentPower25()
{
	return m_runningPower_25W*25*m_wh_per_tick;
}

int PowerCalc::getCurrentPower()
{
	int next_power = m_runningPower_100W*100*m_wh_per_tick;
	int delta = 2*100*m_wh_per_tick;

	int est_power = next_power;

	next_power = m_runningPower_25W*25*m_wh_per_tick;
	if ((next_power > est_power-delta) && (next_power < est_power+delta)) {
		est_power = next_power;
		delta = 2*25*m_wh_per_tick;
	}

	next_power = m_runningPower_5W*5*m_wh_per_tick;
	if ((next_power > est_power-delta) && (next_power < est_power+delta)) {
		est_power = next_power;
		delta = 2*5*m_wh_per_tick;
	}

	return est_power;
}

int PowerCalc::getCurrentEnergy()
{
	return m_runningEnergy*m_wh_per_tick;
}

int PowerCalc::getDeltaSteps()
{
	int ret = ((1<<16)-m_lastDelta+m_last) & ((1<<16)-1);
	m_lastDelta = m_last;
	return ret*m_wh_per_tick;
}

void PowerCalc::resetTotals()
{
	m_runningEnergy = 0;
	m_lastDelta = m_last;
}
