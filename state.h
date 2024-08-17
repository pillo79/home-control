#ifndef __STATE_H__
#define __STATE_H__

#include "trendvalue.h"
#include "ctrlvalue.h"

#include <QMutex>
#include <QSettings>
#include <QThread>
#include <QTime>
#include <QTimer>

class State : private QObject {
		Q_OBJECT
		friend State &s();

		QSettings m_settings;
		QTimer m_saveTimer;

		State();
		virtual ~State() { };

	private slots:
		void do_saveSettings();

	public:
		void loadSettings();
		void saveSettings();

	public:
		QMutex fieldLock;

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
		CtrlBoolVal xTrasfAccumulo;

		CtrlBoolVal xDisabilitaPompaCalore;
		CtrlBoolVal xDisabilitaGas;
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
