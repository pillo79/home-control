#ifndef __STATE_H__
#define __STATE_H__

#include "trendvalue.h"
#include "ctrlvalue.h"

#include <QMutex>
#include <QSettings>
#include <QThread>
#include <QTime>

class State {
		friend State &s();

		State();
		QSettings m_settings;

	public:
		void loadSettings();
		void saveSettings();

	public:
		QMutex mFields;

		/* inputs */
		CtrlBoolVal xModoRiscaldamento;
		CtrlBoolVal xAttivaZonaNotte;
		CtrlBoolVal xAttivaZonaGiorno;
		CtrlBoolVal xAttivaZonaSoffitta;
		CtrlBoolVal xAttivaFanCoil;
		CtrlBoolVal xAttivaProg;

		CtrlBoolVal xSetManuale;
		CtrlBoolVal xUsaPompaCalore;
		CtrlBoolVal xUsaGas;
		CtrlBoolVal xUsaResistenze;
		CtrlBoolVal xTrasfDaAccumulo;
		CtrlBoolVal xTrasfVersoAccumulo;

		CtrlBoolVal xDisabilitaPompaCalore;
		CtrlBoolVal xDisabilitaGas;
		CtrlBoolVal xDisabilitaResistenze;
		CtrlBoolVal xDisabilitaAccumulo;

		CtrlIntVal wVelFanCoil;
		CtrlIntVal wApriCucinaPerc;
		CtrlBoolVal xForzaChiudi;

		/* outputs */
		CtrlBoolVal xGasInUso;
		CtrlBoolVal xLegnaInUso;
		CtrlBoolVal xImpiantoAttivo;
		CtrlBoolVal xPompaCaloreRiscInUso;
		CtrlBoolVal xPompaCaloreCondInUso;
		CtrlBoolVal xPompaCaloreAttiva;
		CtrlBoolVal xResistenzeInUso;
		CtrlBoolVal xTrasfDaAccumuloInCorso;
		CtrlBoolVal xTrasfVersoAccumuloInCorso;

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
