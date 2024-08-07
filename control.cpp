#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"
#include "powercalc.h"
#include "trendbase.h"

#include <QTime>
#include <stdio.h>

#define MAX_PTS 480 // 60*24/3 (1 day in steps of 3 minutes)

struct TargetPower {
	 int power;
	 int min_budget;
	 int io_map[5];
};

const TargetPower POWER_LEVEL[] = {
	{	   0,  -9999,	{ 0, 0, 0, 0, 0 }, },
	{	 900,	1000,	{ 0, 0, 1, 1, 0 }, },
	{	1400,	1600,	{ 0, 1, 0, 1, 0 }, },
	{	1800,	2100,	{ 0, 1, 0, 1, 1 }, },
	{	2500,	2900,	{ 1, 1, 0, 0, 0 }, },
//	{	2500,	2900,	{ 0, 1, 1, 1, 0 }, },	// alternativa
	{	3800,	4300,	{ 1, 1, 0, 1, 0 }, },
	//{	6000,	6500,	{ 1, 1, 0, 0, 1 }, },	// ancora da testare!
};
const int POWER_LEVELS = sizeof(POWER_LEVEL)/sizeof(TargetPower);

ControlThread::ControlThread()
	: wTemperaturaACS	("TemperaturaACS",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTemperaturaBoiler	("TemperaturaBoiler",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTemperaturaAccumulo	("TemperaturaAccumulo",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTemperaturaPannelli	("TemperaturaPannelli",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTempLegnaH		("TempGiorno",		"°C",	"%.1f", MAX_PTS, 0.5)
	, wTempLegnaL		("TempNotte",		"°C",	"%.1f", MAX_PTS, 0.5)

	, wPotProdotta		("PotProdotta",		"W",	"%.0f", MAX_PTS, 0.0)
	, wPotConsumata		("PotConsumata",	"W",	"%.0f", MAX_PTS, 0.0)
	, wPotResistenze	("PotResistenze",	"W",	"%.0f", MAX_PTS, 0.0)
	, wEnergProdotta	("EnergProdotta",	"kWh",	"%.3f", MAX_PTS, 0.0)
	, wEnergConsumata	("EnergConsumata",	"kWh",	"%.3f", MAX_PTS, 0.0)
	, wEnergPassivo		("EnergPassivo",	"kWh",	"%.3f", MAX_PTS, 0.0)

	, wTempEsterno		("TempEsterno",		"°C",	"%.1f", MAX_PTS, 0.5)
{
	wApriCucinaPerc = 40;
	wVelFanCoil = 20;
	xModoRiscaldamento = true;

	TrendBase::instance()->open();

	start();
}

ControlThread::~ControlThread()
{

}

static void echo(const char *what, const char *where)
{
	FILE *f = fopen(where, "w");
	fprintf(f, "%s", what);
	fclose(f);
}

static void append(const char *what, const char *where)
{
	FILE *f = fopen(where, "a");
	fprintf(f, "%s", what);
	fclose(f);
}

void ControlThread::setPowerLevel(int level)
{
	for (int i=0; i<5; ++i)
		HW.PompaCalore.xConfigResistenze[i]->setValue(POWER_LEVEL[level].io_map[i]);

	xResistenzeInUso = (level > 0);
}

void ControlThread::run()
{
	ModbusDevice::openSerial("/dev/ttyUSB0");
	InitHardware();

	QTime lastTime = QTime::currentTime();
	int lastSecs = lastTime.hour()*3600 + lastTime.minute()*60 + lastTime.second();

	bool xAutoGas = false;
	bool xAutoPompaCaloreRisc = false;
	bool xAutoPompaCaloreCond = false;
	bool xAutoTrasfDaAccumulo = false;
	bool xAutoTrasfVersoAccumulo = false;
	bool xCaricoAccumuloAttivo = false;

	bool xRichiestaRestart = false;

	int PowerLevel = 0;
	int NextPowerLevel = -1;

	bool initGiorEnergia = true;
	double giorEnergProdotta = 0.0;
	double giorEnergConsumata = 0.0;

	bool initMinEnergia = true;
	double minEnergProdotta = 0.0;
	double minEnergConsumata = 0.0;

	while(1) {

		QTime rd_time = QTime::currentTime();
		ReadHardwareInputs();
		int rd_ms = rd_time.elapsed();

		mFields.lock();

		wCommErrorMask = GetCommErrorMask();

		// impulso reset PLC
		if (wCommErrorMask) {
			if (!xRichiestaRestart) {
				xRichiestaRestart = true;
				dtLastResetPLC = QDateTime::currentDateTime();
				append(qPrintable(QString("%1 %2\n").arg(dtLastResetPLC.toString("yyyy-MM-dd hh:mm")).arg(wCommErrorMask, 4, 16, QChar('0'))), "/media/mmcblk0p2/reset.log");
				++wResetPLCs;
			}
		}

//		static DelayRiseTimer tRichiestaRestart;
//		bool xRitardoRichiestaRestart = tRichiestaRestart.update(DELAY_SEC(1), xRichiestaRestart);
//		HW.xResetPLC->setValue(xRichiestaRestart && !xRitardoRichiestaRestart);
//		if (xRitardoRichiestaRestart && !wCommErrorMask)
//			xRichiestaRestart = false;

		wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue() / 10.0;
		wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue() / 10.0;
		wTemperaturaAccumulo = HW.Accumulo.wTemperatura->getValue() / 10.0;
		wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue() / 10.0;
		wTempLegnaH = HW.Legna.wTemperaturaPufferH->getValue() / 10.0;
		wTempLegnaL = HW.Legna.wTemperaturaPufferL->getValue() / 10.0;
		xLegnaInUso = HW.Legna.xCaldaiaAccesa->getValue();

		wTempEsterno =  HW.Ambiente.wTemperaturaEsterna->getValue() / 10.0;

		Timer::tick();
		QDateTime today = QDateTime::currentDateTime();
		QTime now = today.time();
		int nowSecs = now.hour()*3600 + now.minute()*60 + now.second();

		wPotProdotta = HW.Pannelli.wPotenzaProdotta->getValue();
		wPotConsumata = HW.Pannelli.wPotenzaConsumata->getValue();

		double curEnergProdotta = HW.Pannelli.wEnergiaProdotta->getValue() / 1000.0;
		double curEnergConsumata = HW.Pannelli.wEnergiaConsumata->getValue() / 1000.0;
		if (initGiorEnergia) {
			giorEnergProdotta = curEnergProdotta;
			giorEnergConsumata = curEnergConsumata;
			initGiorEnergia = false;
		}
		wEnergProdotta = curEnergProdotta - giorEnergProdotta;
		wEnergConsumata = curEnergConsumata - giorEnergConsumata;

		wPotResistenze = HW.Pannelli.wPotenzaResistenze->getValue();

		if (now.minute() != lastTime.minute()) {
			if (initMinEnergia) {
				// do not add data for first cycle
				initMinEnergia = false;
			}  else {
				double prod = curEnergProdotta - minEnergProdotta;
				double cons = curEnergConsumata - minEnergConsumata;
				if (prod > cons)
					tTempoAttivo = tTempoAttivo.addSecs(60);
				else
					wEnergPassivo = wEnergPassivo + (cons-prod);
			}
			minEnergProdotta = curEnergProdotta;
			minEnergConsumata = curEnergConsumata;

			if ((wPotProdotta < 1500) || ((wPotConsumata > wPotProdotta) && !PowerLevel) || (wTemperaturaBoiler > 55))
				xAutoPompaCaloreRisc = false;
			else if ((wPotProdotta > 2500) && (wTemperaturaBoiler < 52))
				xAutoPompaCaloreRisc = true;

			if ((now.minute() % 3) == 0) {
				int power_budget = wPotProdotta + wPotResistenze - wPotConsumata;
				int next_level;

				// never allocate more power than currently produced
				if (power_budget > wPotProdotta)
					power_budget = wPotProdotta;

				for (next_level=POWER_LEVELS-1; next_level>=0; --next_level) {
					if (power_budget > POWER_LEVEL[next_level].min_budget)
						break;
				}

				if (next_level != PowerLevel) {
					NextPowerLevel = next_level;
					setPowerLevel(0);
					printf("%4.0f +%4.0f[%i] -%4.0f = %4i => SWITCH TO %4i[%i]\n",
						wPotProdotta.value(), wPotResistenze.value(), PowerLevel, wPotConsumata.value(),
						power_budget, POWER_LEVEL[NextPowerLevel].power, NextPowerLevel);
				} else {
					printf("%4.0f +%4.0f[%i] -%4.0f = %4i => NO CHANGE\n",
						wPotProdotta.value(), wPotResistenze.value(), PowerLevel, wPotConsumata.value(),
						power_budget);
				}

				// advance TrendValues and save row on DB
				TrendBase::instance()->step(today.toTime_t());
			}
		}

		if (now.hour() != lastTime.hour()) {
			switch (now.hour()) {
			case 0:
				// NEXT DAY!
				tTempoAttivo.setHMS(0,0,0);
				wEnergPassivo = 0.0;
				initGiorEnergia = true;
				xCaricoAccumuloAttivo = false;
				break;
			case 6:
				// wake up display
				echo("160", "/sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");
				break;
			case 22:
				// set display to sleep brightness
				echo("32", "/sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");
				break;
			}
		}

		if (!xModoRiscaldamento) {
			// ignora richieste zone notte/giorno
			xAttivaZonaNotte = xAttivaZonaGiorno = false;
		}

		bool richieste_valide = false;
		if (xAttivaProg) {
			richieste_valide |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			richieste_valide |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		} else {
			richieste_valide = true;
		}

		bool zone_accese = richieste_valide && (xAttivaZonaNotte || xAttivaZonaGiorno || xAttivaZonaSoffitta);
		bool impianto_acceso = richieste_valide && (zone_accese || xAttivaFanCoil);
		bool risc_manuale_solo_hp = xSetManuale && xModoRiscaldamento && xUsaPompaCalore && !xUsaGas;
		xImpiantoAttivo = impianto_acceso;

		HW.Riscaldamento.xChiudiValvola->setValue(!zone_accese);

		static DelayRiseTimer tStartPompe;
		bool start_pompe = tStartPompe.update(DELAY_SEC(20), impianto_acceso);
		HW.Riscaldamento.xStartPompaGiorno->setValue(start_pompe && xAttivaZonaGiorno);
		HW.Riscaldamento.xStartPompaNotte->setValue(start_pompe && xAttivaZonaNotte);
		HW.Riscaldamento.xStartPompaSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilStanzaSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilBagnoSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartPompaCircuito->setValue(start_pompe && xModoRiscaldamento);

		bool acs_attiva = false;
		if (wTemperaturaACS < 55) {
			/* orari ACS auto */
			acs_attiva |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			acs_attiva |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		}

		if (xModoRiscaldamento)
			acs_attiva |= impianto_acceso;

		if (acs_attiva && xDisabilitaGas) {
			// HP deve scaldare ACS anche se potenza non sufficiente
			if (wTemperaturaACS < 50)
				xAutoPompaCaloreRisc = true;
			else if (wTemperaturaACS > 55)
				xAutoPompaCaloreRisc = false;
		}

		if (!xModoRiscaldamento && impianto_acceso) {
			// imposta e forza raffreddamento HP
			xAutoPompaCaloreRisc = false;
			xAutoPompaCaloreCond = true;
		} else {
			xAutoPompaCaloreCond = false;
		}

		// setup valvole pompa calore
		HW.PompaCalore.xForzaValvole->setValue(impianto_acceso);

		// *** 3UVB (miscelatrice)
		// 3Vie (risc o acs):	chiusa
		// 3Vie (condiz):	aperta
		HW.PompaCalore.xForza3VieApri->setValue(impianto_acceso && (xAutoPompaCaloreCond || risc_manuale_solo_hp));
		HW.PompaCalore.xForza3VieChiudi->setValue(impianto_acceso && !(xAutoPompaCaloreCond || risc_manuale_solo_hp));

		// *** 3UV1 (movimento lungo, quella che si ferma a 3/4)
		// risc o ACS:			3/4 (non alimentata + burst)
		// risc solo con pompa calore:	chiusa (non alimentata)
		// condiz:			chiusa (non alimentata)

		// 20 secondi = 8 secondi di tempo per riportare a fondo scala
		// moltiplicato 2.5* per permettere al motore di andare
		// *dall'altra parte* se aveva iniziato la fase di chiusura
		static DelayRiseTimer tResetManValvole;
		bool reset_man_finito = tResetManValvole.update(DELAY_SEC(20), impianto_acceso);
		static DelayRiseTimer tSetPosValvolaUV1;
		bool set_pos_uv1_finito = tSetPosValvolaUV1.update(DELAY_MSEC(1900), reset_man_finito);
		HW.PompaCalore.xForzaRiscApri->setValue(impianto_acceso && xModoRiscaldamento && !reset_man_finito && !risc_manuale_solo_hp);
		HW.PompaCalore.xForzaRiscFerma->setValue(xModoRiscaldamento && set_pos_uv1_finito && !risc_manuale_solo_hp);

		if (xDisabilitaPompaCalore) {
			// disabilita comando a pompa calore ma gestisci valvole
			xAutoPompaCaloreRisc = xAutoPompaCaloreCond = false;
		}

		static DelayRiseTimer tNuovoLivelloRes;
		if (xDisabilitaResistenze || (wTemperaturaACS > 80) || (wTemperaturaBoiler > 80) || (wPotConsumata < 0.1)) {
			// force off
			setPowerLevel(0);
			PowerLevel = 0;
			NextPowerLevel = -1;
		} else if (tNuovoLivelloRes.update(DELAY_MSEC(500), (NextPowerLevel != -1))) {
			// update!
			setPowerLevel(NextPowerLevel);
			PowerLevel = NextPowerLevel;
			NextPowerLevel = -1;
		}

		xPompaCaloreAttiva = HW.PompaCalore.xStatoPompaCalore->getValue();

		HW.FanCoilCorridoio.xChiudiValvola->setValue(!xAttivaFanCoil);
		static DelayRiseTimer tStartFanCoil;
		bool valvola_fancoil_chiusa = tStartFanCoil.update(DELAY_SEC(30), xAttivaFanCoil);

		// filtro ventilatore per riscaldamento manuale solo HP
		static bool ventilatore_in_funzione = false;
		static DelayRiseTimer tStartVentilatore;
		bool start_ventilatore = tStartVentilatore.update(DELAY_SEC(60), xPompaCaloreAttiva);
		static DelayFallTimer tStopVentilatore;
		bool stop_ventilatore = tStopVentilatore.update(DELAY_SEC(120), xPompaCaloreAttiva);
		if (start_ventilatore || !risc_manuale_solo_hp)
			ventilatore_in_funzione = true;
		else if (!stop_ventilatore)
			ventilatore_in_funzione = false;

		HW.FanCoilCorridoio.xStartVentilatore->setValue(valvola_fancoil_chiusa && ventilatore_in_funzione);
		HW.FanCoilCorridoio.wLivelloVentilatore->setValue(wVelFanCoil*200);

		static bool set_serranda_cucina = false;

		static bool ultimo_fancoil = false;
		if (ultimo_fancoil != xAttivaFanCoil)
			set_serranda_cucina = true;
		ultimo_fancoil = xAttivaFanCoil;

		static int ultimo_perc_cucina = 0;
		if (ultimo_perc_cucina != wApriCucinaPerc)
			set_serranda_cucina = true;
		ultimo_perc_cucina = wApriCucinaPerc;

		static bool ultimo_forza_chiudi = false;
		if (ultimo_forza_chiudi != xForzaChiudi)
			set_serranda_cucina = true;
		ultimo_forza_chiudi = xForzaChiudi;

		static DelayRiseTimer tResetSerrandaCucina;
		static DelayRiseTimer tApriSerrandaCucina;

		bool reset_cucina_ok = tResetSerrandaCucina.update(DELAY_SEC(30), set_serranda_cucina);
		bool chiudi_cucina = set_serranda_cucina && !reset_cucina_ok;

		bool cucina_aperta = tApriSerrandaCucina.update(DELAY_SEC((wApriCucinaPerc*25)/100), reset_cucina_ok);
		bool apri_cucina = reset_cucina_ok && xAttivaFanCoil && !xForzaChiudi && !cucina_aperta;

		bool muovi_serranda = apri_cucina || chiudi_cucina;
		if (set_serranda_cucina && !muovi_serranda)
			set_serranda_cucina = false;

		HW.FanCoilCorridoio.xMuoviSerrandaCucina->setValue(muovi_serranda);
		HW.FanCoilCorridoio.xApriSerrandaCucina->setValue(apri_cucina);

		if (xSetManuale) {
			// forza pompa off se sovra temp
			if (xUsaPompaCalore && xModoRiscaldamento && (wTemperaturaACS > 65))
				xUsaPompaCalore = false;

			xPompaCaloreRiscInUso = xUsaPompaCalore && xModoRiscaldamento;
			xPompaCaloreCondInUso = xUsaPompaCalore && !xModoRiscaldamento;
		} else {
			xPompaCaloreRiscInUso = xAutoPompaCaloreRisc;
			xPompaCaloreCondInUso = xAutoPompaCaloreCond;
		}

		HW.PompaCalore.xStopPompaCalore->setValue(!(xPompaCaloreRiscInUso || xPompaCaloreCondInUso));
		HW.PompaCalore.xRichiestaCaldo->setValue(xPompaCaloreRiscInUso);
		HW.PompaCalore.xRichiestaFreddo->setValue(xPompaCaloreCondInUso);

		// condizioni HPSU->cantina ("troppo caldo")
		static bool forzaTrasfHPSUVersoLegna = false;
		if ((wTemperaturaACS > 70) && (wTemperaturaACS > wTempLegnaH+10.0)) {
			forzaTrasfHPSUVersoLegna = true;
		} else if ((wTemperaturaACS < 65) || (wTemperaturaACS < wTempLegnaH+6.0)) {
			forzaTrasfHPSUVersoLegna = false;
		}
		static bool equilibraLegna = false;
		if ((wTempLegnaH < wTempLegnaL+5.0) || !forzaTrasfHPSUVersoLegna)
			equilibraLegna = false;
		else if (wTempLegnaH > wTempLegnaL+20.0)
			equilibraLegna = true;
		HW.Legna.xForzaP2Scambiatore->setValue(forzaTrasfHPSUVersoLegna);
		HW.Legna.xStartP2Scambiatore->setValue(forzaTrasfHPSUVersoLegna);
		HW.Legna.xStartP4RicircoloEstate->setValue(equilibraLegna);

		// condizioni HPSU->accumulo ("salvataggio energia")
		// abilitato quando tBoiler > 60C
		// disabilitato ogni giorno alle 18
		bool orario_carico_accumulo = (now>QTime(10,0)) && (now<QTime(18,0));
		bool orario_uso_accumulo = (now>QTime(7,0)) && (now<QTime(22,0));
		bool ferma_accumulo = true;

		if (!orario_carico_accumulo)
			xCaricoAccumuloAttivo = false;
		else if (wTemperaturaBoiler > 60)
			xCaricoAccumuloAttivo = true;

		if ((orario_uso_accumulo || acs_attiva) && !xDisabilitaAccumulo) {
			/* trasf Accumulo->HPSU (uso energia) */
			if ((wTemperaturaAccumulo > 40) && (wTemperaturaAccumulo > wTemperaturaACS+6.0) && !xGasInUso) {
				xAutoTrasfDaAccumulo = true;
				xAutoTrasfVersoAccumulo = false;
			}

			/* situazione accumulo gestita */
			ferma_accumulo = false;
		}

		if (!xAutoTrasfDaAccumulo && xCaricoAccumuloAttivo && !xDisabilitaAccumulo) {
			/* trasf HPSU->Accumulo (salvataggio energia)*/
			if ((wTemperaturaACS > 50) && ((wTemperaturaAccumulo < wTemperaturaACS-6) || (wTemperaturaACS > 70)) && !xGasInUso) {
				xAutoTrasfDaAccumulo = false;
				xAutoTrasfVersoAccumulo = true;
			}

			/* situazione accumulo gestita */
			ferma_accumulo = false;
		}

		if (ferma_accumulo || xGasInUso || (wTemperaturaAccumulo < 35) || (wTemperaturaAccumulo < wTemperaturaACS+2)) {
			/* stop trasf Accumulo->HPSU (uso energia) */
			xAutoTrasfDaAccumulo = false;
		}

		if (ferma_accumulo || xGasInUso || (wTemperaturaACS < 45) || ((wTemperaturaACS < 69) && (wTemperaturaAccumulo > wTemperaturaACS-2))) {
			/* stop trasf HPSU->Accumulo (salvataggio energia)*/
			xAutoTrasfVersoAccumulo = false;
		}

		/* se trasf Accumulo->HPSU attivo -> no caldaia */
		if (xAutoTrasfDaAccumulo)
			acs_attiva = false;

		if (xSetManuale) {
			xTrasfDaAccumuloInCorso = xTrasfDaAccumulo;
			xTrasfVersoAccumuloInCorso = xTrasfVersoAccumulo;
		} else {
			xTrasfDaAccumuloInCorso = xAutoTrasfDaAccumulo;
			xTrasfVersoAccumuloInCorso = xAutoTrasfVersoAccumulo;
		}

		bool start_pompa_accumulo = (xTrasfDaAccumuloInCorso || xTrasfVersoAccumuloInCorso);
		static DelayRiseTimer tDurataPompaAccumulo;
		bool max_durata_pompa_accumulo = tDurataPompaAccumulo.update(DELAY_MIN(120), start_pompa_accumulo && xSetManuale);
		HW.Accumulo.xStartPompa->setValue(start_pompa_accumulo && !max_durata_pompa_accumulo);
		if (max_durata_pompa_accumulo) {
			xTrasfDaAccumuloInCorso = false;
			xTrasfVersoAccumuloInCorso = false;
		}

		if (acs_attiva && !xPompaCaloreRiscInUso && !xDisabilitaGas) {
			/* auto mode */
			if (wTemperaturaACS < 50)
				xAutoGas = true;
		else if (wTemperaturaACS > 55)
			xAutoGas = false;
		} else {
			xAutoGas = false;
		}

		if (xSetManuale)
			xGasInUso = xUsaGas;
		else
			xGasInUso = xAutoGas;

		HW.Gas.xAlimenta->setValue(xGasInUso);
		static DelayRiseTimer tStartGas;
		HW.Gas.xStartCaldaia->setValue(tStartGas.update(DELAY_SEC(10), xGasInUso));
		static DelayFallTimer tStartPompa;
		HW.Gas.xStartPompa->setValue(tStartPompa.update(DELAY_SEC(60), HW.Gas.xStartCaldaia->getValue()));

		mFields.unlock();
		int ctrl_ms = now.elapsed();
		QTime wr_time = QTime::currentTime();
		WriteHardwareOutputs();
		int wr_ms = wr_time.elapsed();

		static int cycle = 0;
		if ((rd_ms >= 100) || (wr_ms >= 100) || !(cycle & 0xff))
			printf("cycle: %08x rd %4i ctrl %i wr %4i\n", cycle, rd_ms, ctrl_ms, wr_ms);
		lastTime = now;
		lastSecs = nowSecs;
		cycle++;
	}
}

ControlThread &control()
{
	static ControlThread theControl;
	return theControl;
}
