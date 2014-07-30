#include "powercalc.h"

#include <stdio.h>
#include <string.h>

#define SAMPLES_PER_HOUR	1100

#define WRAP(x) ((x+SAMPLES_PER_HOUR) % SAMPLES_PER_HOUR)

PowerCalc::PowerCalc()
	: m_samples(new int[SAMPLES_PER_HOUR])
	, m_initLast(true)
{
	bzero(m_samples, SAMPLES_PER_HOUR*sizeof(int));
	printf("inited\n");
}

PowerCalc::~PowerCalc()
{
	delete m_samples;
}

void PowerCalc::addSample(int now)
{
	if (m_initLast) {
		m_last = now;
		m_initLast = false;
		return;
	}

	unsigned long wrappedValue = ((1UL<<31)-m_last+now) & ((1<<31)-1);
	m_runningPower_1W += wrappedValue-m_samples[m_sampleIdx];
	m_runningPower_5W += wrappedValue-m_samples[WRAP(m_sampleIdx-SAMPLES_PER_HOUR/5)];
	m_runningPower_25W += wrappedValue-m_samples[WRAP(m_sampleIdx-SAMPLES_PER_HOUR/25)];
	m_runningPower_100W += wrappedValue-m_samples[WRAP(m_sampleIdx-SAMPLES_PER_HOUR/100)];
	m_runningEnergy += wrappedValue;
	printf("%3i now %04x last %04x wrap %04x totals %5i %04x %5i %04x %5i %04x %5i %04x %04x\n", m_sampleIdx, now, m_last, wrappedValue, m_runningPower_1W, m_runningPower_1W, m_runningPower_5W*5, m_runningPower_5W, m_runningPower_25W*25, m_runningPower_25W, m_runningPower_100W*100, m_runningPower_100W, m_runningEnergy);
	m_samples[m_sampleIdx] = wrappedValue;
	m_sampleIdx = WRAP(m_sampleIdx+1);
	m_last = now;
}

int PowerCalc::getCurrentPower()
{
	int next_power = m_runningPower_100W*100;
	int delta = 2*100;

	int est_power = next_power;

	next_power = m_runningPower_25W*25;
	if ((next_power > est_power-delta) && (next_power < est_power+delta)) {
		est_power = next_power;
		delta = 2*25;
	}

	next_power = m_runningPower_5W*5;
	if ((next_power > est_power-delta) && (next_power < est_power+delta)) {
		est_power = next_power;
		delta = 2*5;
	}

	next_power = m_runningPower_1W;
	if ((next_power > est_power-delta) && (next_power < est_power+delta)) {
		est_power = next_power;
	}

	return est_power;
}

int PowerCalc::getCurrentEnergy()
{
	return m_runningEnergy;
}

void PowerCalc::resetTotals()
{
	m_runningEnergy = 0;
}
