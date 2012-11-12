#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QThread>
#include <QMutex>

class ControlThread : public QThread {
	private:
		void run();
	public:
		ControlThread();
		~ControlThread();

		QMutex mFields;

		/* inputs */
		bool xRiscaldaNotte;
		bool xRiscaldaGiorno;
		bool xRiscaldaSoffitta;
		bool xFanCoil;

		bool xUsaPompaCalore;
		bool xUsaCaldaia;

		bool xTrasfAccumulo;
		bool xApriCucina;
		bool xChiudiCucina;

		int wVelFanCoil;

		/* outputs */
		bool xCaldaiaInUso;

		int wTemperaturaACS;
		int wTemperaturaBoiler;
		int wTemperaturaAccumuli;
};

ControlThread &control();

#endif /* __CONTROL_H__ */
