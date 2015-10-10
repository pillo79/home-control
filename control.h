#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QThread>
#include <QMutex>
#include <QTime>

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
		bool xUsaResistenze;

		bool xTrasfAccumulo;
		bool xApriCucina;
		bool xChiudiCucina;

		int wVelFanCoil;

		/* outputs */
		bool xCaldaiaInUso;
		bool xPompaCaloreInUso;
		bool xResistenzeInUso;
		bool xTrasfAccumuloInCorso;

		int wTemperaturaACS;
		int wTemperaturaBoiler;
		int wTemperaturaAccumulo;
		int wTemperaturaPannelli;

		int wPotProdotta;
		int wPotConsumata;
		int wEnergProdotta;
		int wEnergConsumata;
		QTime tTempoAttivo;

		int wTempGiorno;
		int wUmidGiorno;
		int wTempNotte;
		int wUmidNotte;
		int wTempSoffitta;
		int wUmidSoffitta;
		int wTempEsterno;
		int wUmidEsterno;
};

ControlThread &control();

#endif /* __CONTROL_H__ */
