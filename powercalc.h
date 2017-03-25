#ifndef __POWERCALC_H__
#define __POWERCALC_H__

#define SAMPLE_PERIOD_SECS	3
#define SAMPLES_PER_HOUR	(3600/SAMPLE_PERIOD_SECS)	// keep it integer

class PowerCalc {
	public:
		PowerCalc();
		~PowerCalc();

		void addSample(int now);
		void restart();
		void resetTotals();

		int getCurrentPower();
		int getCurrentPower25();
		int getCurrentEnergy();
		int getDeltaSteps();

	private:
		int* m_samples;
		int m_last_5W[8], m_last_25W[8], m_last_100W[8];
		int m_sampleIdx;
		bool m_initLast;
		int m_last;
		int m_lastDelta;
		int m_runningPower_5W, m_maPower_5W;
		int m_runningPower_25W, m_maPower_25W;
		int m_runningPower_100W, m_maPower_100W;
		int m_runningEnergy;
};

#endif /* __POWERCALC_H__ */
