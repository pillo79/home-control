#ifndef __STATE_H__
#define __STATE_H__

#include "trendvalue.h"

#include <QThread>
#include <QMutex>
#include <QTime>

class State {
		friend State &s();

		State();

	public:

		QMutex mFields;

		/* inputs */
		bool xModoRiscaldamento;
		bool xAttivaZonaNotte;
		bool xAttivaZonaGiorno;
		bool xAttivaZonaSoffitta;
		bool xAttivaFanCoil;
		bool xAttivaProg;

		bool xSetManuale;
		bool xUsaPompaCalore;
		bool xUsaGas;
		bool xUsaResistenze;
		bool xTrasfDaAccumulo;
		bool xTrasfVersoAccumulo;

		bool xDisabilitaPompaCalore;
		bool xDisabilitaGas;
		bool xDisabilitaResistenze;
		bool xDisabilitaAccumulo;

		int wVelFanCoil;
		int wApriCucinaPerc;
		bool xForzaChiudi;

		/* outputs */
		bool xGasInUso;
		bool xLegnaInUso;
		bool xImpiantoAttivo;
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
		TrendValue wTempLegnaH;
		TrendValue wTempLegnaL;

		TrendValue wPotProdotta;
		TrendValue wPotConsumata;
		TrendValue wPotResistenze;
		TrendValue wEnergProdotta;
		TrendValue wEnergConsumata;
		QTime tTempoAttivo;
		TrendValue wEnergPassivo;

		TrendValue wTempEsterno;

		uint32_t wCommErrorMask;
		uint32_t wResetPLCs;
		QDateTime dtLastResetPLC;
};

State &s();

#endif /* __STATE_H__ */
