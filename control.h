#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QThread>
#include <QMutex>
#include <QTime>

#include "trendvalue.h"

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
		bool xPompaCaloreAttiva;
		bool xResistenzeInUso;
		bool xTrasfDaAccumuloInCorso;
		bool xTrasfVersoAccumuloInCorso;

		TrendValue wTemperaturaACS;
		TrendValue wTemperaturaBoiler;
		TrendValue wTemperaturaAccumulo;
		TrendValue wTemperaturaPannelli;

		TrendValue wPotProdotta;
		TrendValue wPotConsumata;
		TrendValue wPotResistenze;
		TrendValue wEnergProdotta;
		TrendValue wEnergConsumata;
		QTime tTempoAttivo;
		TrendValue wEnergPassivo;

		TrendValue wTempGiorno;
		TrendValue wUmidGiorno;
		TrendValue wTempNotte;
		TrendValue wUmidNotte;
		TrendValue wTempSoffitta;
		TrendValue wUmidSoffitta;
		TrendValue wTempEsterno;
		TrendValue wUmidEsterno;

		uint32_t wCommErrorMask;
		uint32_t wResetPLCs;
		QDateTime dtLastResetPLC;
};

ControlThread &control();

#endif /* __CONTROL_H__ */
