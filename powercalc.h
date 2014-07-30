#ifndef __POWERCALC_H__
#define __POWERCALC_H__

class PowerCalc {
	public:
		PowerCalc();
		~PowerCalc();

		void addSample(int now);
		void resetTotals();

		int getCurrentPower();
		int getCurrentEnergy();

	private:
		int* m_samples;
		int m_sampleIdx;
		bool m_initLast;
		int m_last;
		int m_runningPower_1W;
		int m_runningPower_5W;
		int m_runningPower_25W;
		int m_runningPower_100W;
		int m_runningEnergy;
};

#endif /* __POWERCALC_H__ */
