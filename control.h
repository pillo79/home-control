#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QThread>
#include <QMutex>
#include <QTime>

class ControlThread : public QThread {
	private:
		void setPowerLevel(int level);
		void run();
	public:
		ControlThread();
		~ControlThread();

		QMutex mFields;

		/* inputs */
		bool xModoRiscaldamento;
		bool xAttivaZonaNotte;
		bool xAttivaZonaGiorno;
		bool xAttivaZonaSoffitta;
		bool xAttivaFanCoil;

		bool xSetManuale;
		bool xUsaPompaCalore;
		bool xUsaCaldaia;
		bool xUsaResistenze;
		bool xTrasfDaAccumulo;
		bool xTrasfVersoAccumulo;

		bool xDisabilitaPompaCalore;
		bool xDisabilitaCaldaia;
		bool xDisabilitaResistenze;
		bool xDisabilitaAccumulo;

		bool xApriCucina;
		bool xChiudiCucina;

		int wVelFanCoil;

		/* outputs */
		bool xCaldaiaInUso;
		bool xPompaCaloreRiscInUso;
		bool xPompaCaloreCondInUso;
		bool xResistenzeInUso;
		bool xTrasfDaAccumuloInCorso;
		bool xTrasfVersoAccumuloInCorso;

		int wTemperaturaACS;
		int wTemperaturaBoiler;
		int wTemperaturaAccumulo;
		int wTemperaturaPannelli;

		int wPotProdotta;
		int wPotConsumata;
		int wPotResistenze;
		int wEnergProdotta;
		int wEnergConsumata;
		QTime tTempoAttivo;
		int wEnergPassivo;

		int wTempGiorno;
		int wUmidGiorno;
		int wTempNotte;
		int wUmidNotte;
		int wTempSoffitta;
		int wUmidSoffitta;
		int wTempEsterno;
		int wUmidEsterno;

		uint32_t wCommErrorMask;
};

ControlThread &control();

#endif /* __CONTROL_H__ */
