#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"
#include "powercalc.h"
#include "trendbase.h"

#include "ctrlvalue.h"
#include "ctrlobserver.h"

#include "state.h"

#include <QTime>
#include <stdio.h>

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
{
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

	s().xResistenzeInUso(O_CTRL) = (level > 0);
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

		s().fieldLock.lock();

		s().wCommErrorMask = GetCommErrorMask();

		// impulso reset PLC
		if (s().wCommErrorMask) {
			if (!xRichiestaRestart) {
				xRichiestaRestart = true;
				s().dtLastResetPLC = QDateTime::currentDateTime();
				append(qPrintable(QString("%1 %2\n").arg(s().dtLastResetPLC.toString("yyyy-MM-dd hh:mm")).arg(s().wCommErrorMask, 4, 16, QChar('0'))), "/media/mmcblk0p2/reset.log");
				++s().wResetPLCs;
			}
		}

//		static DelayRiseTimer tRichiestaRestart;
//		bool xRitardoRichiestaRestart = tRichiestaRestart.update(DELAY_SEC(1), xRichiestaRestart);
//		HW.xResetPLC->setValue(xRichiestaRestart && !xRitardoRichiestaRestart);
//		if (xRitardoRichiestaRestart && !s().wCommErrorMask)
//			xRichiestaRestart = false;

		s().wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue() / 10.0;
		s().wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue() / 10.0;
		s().wTemperaturaAccumulo = HW.Accumulo.wTemperatura->getValue() / 10.0;
		s().wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue() / 10.0;
		s().wTempLegnaH = HW.Legna.wTemperaturaPufferH->getValue() / 10.0;
		s().wTempLegnaL = HW.Legna.wTemperaturaPufferL->getValue() / 10.0;
		s().xLegnaInUso(O_CTRL) = HW.Legna.xCaldaiaAccesa->getValue();

		s().wTempEsterno =  HW.Ambiente.wTemperaturaEsterna->getValue() / 10.0;

		Timer::tick();
		QDateTime today = QDateTime::currentDateTime();
		QTime now = today.time();
		int nowSecs = now.hour()*3600 + now.minute()*60 + now.second();

		s().wPotProdotta = HW.Pannelli.wPotenzaProdotta->getValue();
		s().wPotConsumata = HW.Pannelli.wPotenzaConsumata->getValue();

		double curEnergProdotta = HW.Pannelli.wEnergiaProdotta->getValue() / 1000.0;
		double curEnergConsumata = HW.Pannelli.wEnergiaConsumata->getValue() / 1000.0;
		if (initGiorEnergia) {
			giorEnergProdotta = curEnergProdotta;
			giorEnergConsumata = curEnergConsumata;
			initGiorEnergia = false;
		}
		s().wEnergProdotta = curEnergProdotta - giorEnergProdotta;
		s().wEnergConsumata = curEnergConsumata - giorEnergConsumata;

		s().wPotResistenze = HW.Pannelli.wPotenzaResistenze->getValue();

		if (now.minute() != lastTime.minute()) {
			if (initMinEnergia) {
				// do not add data for first cycle
				initMinEnergia = false;
			}  else {
				double prod = curEnergProdotta - minEnergProdotta;
				double cons = curEnergConsumata - minEnergConsumata;
				if (prod > cons)
					s().tTempoAttivo = s().tTempoAttivo.addSecs(60);
				else
					s().wEnergPassivo = s().wEnergPassivo + (cons-prod);
			}
			minEnergProdotta = curEnergProdotta;
			minEnergConsumata = curEnergConsumata;

			if ((s().wPotProdotta < 1500) || ((s().wPotConsumata > s().wPotProdotta) && !PowerLevel) || (s().wTemperaturaBoiler > 55))
				xAutoPompaCaloreRisc = false;
			else if ((s().wPotProdotta > 2500) && (s().wTemperaturaBoiler < 52))
				xAutoPompaCaloreRisc = true;

			if ((now.minute() % 3) == 0) {
				int power_budget = s().wPotProdotta + s().wPotResistenze - s().wPotConsumata;
				int next_level;

				// never allocate more power than currently produced
				if (power_budget > s().wPotProdotta)
					power_budget = s().wPotProdotta;

				for (next_level=POWER_LEVELS-1; next_level>=0; --next_level) {
					if (power_budget > POWER_LEVEL[next_level].min_budget)
						break;
				}

				if (next_level != PowerLevel) {
					NextPowerLevel = next_level;
					setPowerLevel(0);
					printf("%4.0f +%4.0f[%i] -%4.0f = %4i => SWITCH TO %4i[%i]\n",
						s().wPotProdotta.value(), s().wPotResistenze.value(), PowerLevel, s().wPotConsumata.value(),
						power_budget, POWER_LEVEL[NextPowerLevel].power, NextPowerLevel);
				} else {
					printf("%4.0f +%4.0f[%i] -%4.0f = %4i => NO CHANGE\n",
						s().wPotProdotta.value(), s().wPotResistenze.value(), PowerLevel, s().wPotConsumata.value(),
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
				s().tTempoAttivo.setHMS(0,0,0);
				s().wEnergPassivo = 0.0;
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

		if (!s().xModoRiscaldamento) {
			// ignora richieste zone notte/giorno
			s().xAttivaZonaNotte(O_CTRL) = s().xAttivaZonaGiorno(O_CTRL) = false;
		}

		bool richieste_valide = false;
		if (s().xAttivaProg) {
			richieste_valide |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			richieste_valide |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		} else {
			richieste_valide = true;
		}

		bool zone_accese = richieste_valide && (s().xAttivaZonaNotte || s().xAttivaZonaGiorno || s().xAttivaZonaSoffitta);
		bool impianto_acceso = richieste_valide && (zone_accese || s().xAttivaFanCoil);
		bool risc_manuale_solo_hp = s().xSetManuale && s().xModoRiscaldamento && s().xUsaPompaCalore && !s().xUsaGas;
		s().xImpiantoAttivo(O_CTRL) = impianto_acceso;

		HW.Riscaldamento.xChiudiValvola->setValue(!zone_accese);

		static DelayRiseTimer tStartPompe;
		bool start_pompe = tStartPompe.update(DELAY_SEC(20), impianto_acceso);
		HW.Riscaldamento.xStartPompaGiorno->setValue(start_pompe && s().xAttivaZonaGiorno);
		HW.Riscaldamento.xStartPompaNotte->setValue(start_pompe && s().xAttivaZonaNotte);
		HW.Riscaldamento.xStartPompaSoffitta->setValue(start_pompe && s().xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilStanzaSoffitta->setValue(start_pompe && s().xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilBagnoSoffitta->setValue(start_pompe && s().xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartPompaCircuito->setValue(start_pompe && s().xModoRiscaldamento);

		bool acs_attiva = false;
		if (s().wTemperaturaACS < 55) {
			/* orari ACS auto */
			acs_attiva |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			acs_attiva |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		}

		if (s().xModoRiscaldamento)
			acs_attiva |= impianto_acceso;

		if (acs_attiva && s().xDisabilitaGas) {
			// HP deve scaldare ACS anche se potenza non sufficiente
			if (s().wTemperaturaACS < 50)
				xAutoPompaCaloreRisc = true;
			else if (s().wTemperaturaACS > 55)
				xAutoPompaCaloreRisc = false;
		}

		if (!s().xModoRiscaldamento && impianto_acceso) {
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
		HW.PompaCalore.xForzaRiscApri->setValue(impianto_acceso && s().xModoRiscaldamento && !reset_man_finito && !risc_manuale_solo_hp);
		HW.PompaCalore.xForzaRiscFerma->setValue(s().xModoRiscaldamento && set_pos_uv1_finito && !risc_manuale_solo_hp);

		if (s().xDisabilitaPompaCalore) {
			// disabilita comando a pompa calore ma gestisci valvole
			xAutoPompaCaloreRisc = xAutoPompaCaloreCond = false;
		}

		static DelayRiseTimer tNuovoLivelloRes;
		if (s().xDisabilitaResistenze || (s().wTemperaturaACS > 80) || (s().wTemperaturaBoiler > 80) || (s().wPotConsumata < 0.1)) {
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

		s().xPompaCaloreAttiva(O_CTRL) = HW.PompaCalore.xStatoPompaCalore->getValue();

		HW.FanCoilCorridoio.xChiudiValvola->setValue(!s().xAttivaFanCoil);
		static DelayRiseTimer tStartFanCoil;
		bool valvola_fancoil_chiusa = tStartFanCoil.update(DELAY_SEC(30), s().xAttivaFanCoil);

		// filtro ventilatore per riscaldamento manuale solo HP
		static bool ventilatore_in_funzione = false;
		static DelayRiseTimer tStartVentilatore;
		bool start_ventilatore = tStartVentilatore.update(DELAY_SEC(60), s().xPompaCaloreAttiva);
		static DelayFallTimer tStopVentilatore;
		bool stop_ventilatore = tStopVentilatore.update(DELAY_SEC(120), s().xPompaCaloreAttiva);
		if (start_ventilatore || !risc_manuale_solo_hp)
			ventilatore_in_funzione = true;
		else if (!stop_ventilatore)
			ventilatore_in_funzione = false;

		HW.FanCoilCorridoio.xStartVentilatore->setValue(valvola_fancoil_chiusa && ventilatore_in_funzione);
		HW.FanCoilCorridoio.wLivelloVentilatore->setValue(s().wVelFanCoil*200);

		static bool set_serranda_cucina = false;

		static bool ultimo_fancoil = false;
		if (ultimo_fancoil != s().xAttivaFanCoil)
			set_serranda_cucina = true;
		ultimo_fancoil = s().xAttivaFanCoil;

		static int ultimo_perc_cucina = 0;
		if (ultimo_perc_cucina != s().wApriCucinaPerc)
			set_serranda_cucina = true;
		ultimo_perc_cucina = s().wApriCucinaPerc;

		static bool ultimo_forza_chiudi = false;
		if (ultimo_forza_chiudi != s().xForzaChiudi)
			set_serranda_cucina = true;
		ultimo_forza_chiudi = s().xForzaChiudi;

		static DelayRiseTimer tResetSerrandaCucina;
		static DelayRiseTimer tApriSerrandaCucina;

		bool reset_cucina_ok = tResetSerrandaCucina.update(DELAY_SEC(30), set_serranda_cucina);
		bool chiudi_cucina = set_serranda_cucina && !reset_cucina_ok;

		bool cucina_aperta = tApriSerrandaCucina.update(DELAY_SEC((s().wApriCucinaPerc*25)/100), reset_cucina_ok);
		bool apri_cucina = reset_cucina_ok && s().xAttivaFanCoil && !s().xForzaChiudi && !cucina_aperta;

		bool muovi_serranda = apri_cucina || chiudi_cucina;
		if (set_serranda_cucina && !muovi_serranda)
			set_serranda_cucina = false;

		HW.FanCoilCorridoio.xMuoviSerrandaCucina->setValue(muovi_serranda);
		HW.FanCoilCorridoio.xApriSerrandaCucina->setValue(apri_cucina);

		if (s().xSetManuale) {
			// forza pompa off se sovra temp
			if (s().xUsaPompaCalore && s().xModoRiscaldamento && (s().wTemperaturaACS > 65))
				s().xUsaPompaCalore(O_CTRL) = false;

			s().xPompaCaloreRiscInUso(O_CTRL) = s().xUsaPompaCalore && s().xModoRiscaldamento;
			s().xPompaCaloreCondInUso(O_CTRL) = s().xUsaPompaCalore && !s().xModoRiscaldamento;
		} else {
			s().xPompaCaloreRiscInUso(O_CTRL) = xAutoPompaCaloreRisc;
			s().xPompaCaloreCondInUso(O_CTRL) = xAutoPompaCaloreCond;
		}

		HW.PompaCalore.xStopPompaCalore->setValue(!(s().xPompaCaloreRiscInUso || s().xPompaCaloreCondInUso));
		HW.PompaCalore.xRichiestaCaldo->setValue(s().xPompaCaloreRiscInUso);
		HW.PompaCalore.xRichiestaFreddo->setValue(s().xPompaCaloreCondInUso);

		// condizioni HPSU->cantina ("troppo caldo")
		static bool forzaTrasfHPSUVersoLegna = false;
		if ((s().wTemperaturaACS > 70) && (s().wTemperaturaACS > s().wTempLegnaH+10.0)) {
			forzaTrasfHPSUVersoLegna = true;
		} else if ((s().wTemperaturaACS < 65) || (s().wTemperaturaACS < s().wTempLegnaH+6.0)) {
			forzaTrasfHPSUVersoLegna = false;
		}
		static bool equilibraLegna = false;
		if ((s().wTempLegnaH < s().wTempLegnaL+5.0) || !forzaTrasfHPSUVersoLegna)
			equilibraLegna = false;
		else if (s().wTempLegnaH > s().wTempLegnaL+20.0)
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
		else if (s().wTemperaturaBoiler > 60)
			xCaricoAccumuloAttivo = true;

		if ((orario_uso_accumulo || acs_attiva) && !s().xDisabilitaAccumulo) {
			/* trasf Accumulo->HPSU (uso energia) */
			if ((s().wTemperaturaAccumulo > 40) && (s().wTemperaturaAccumulo > s().wTemperaturaACS+6.0) && !s().xGasInUso) {
				xAutoTrasfDaAccumulo = true;
				xAutoTrasfVersoAccumulo = false;
			}

			/* situazione accumulo gestita */
			ferma_accumulo = false;
		}

		if (!xAutoTrasfDaAccumulo && xCaricoAccumuloAttivo && !s().xDisabilitaAccumulo) {
			/* trasf HPSU->Accumulo (salvataggio energia)*/
			if ((s().wTemperaturaACS > 50) && ((s().wTemperaturaAccumulo < s().wTemperaturaACS-6) || (s().wTemperaturaACS > 70)) && !s().xGasInUso) {
				xAutoTrasfDaAccumulo = false;
				xAutoTrasfVersoAccumulo = true;
			}

			/* situazione accumulo gestita */
			ferma_accumulo = false;
		}

		if (ferma_accumulo || s().xGasInUso || (s().wTemperaturaAccumulo < 35) || (s().wTemperaturaAccumulo < s().wTemperaturaACS+2)) {
			/* stop trasf Accumulo->HPSU (uso energia) */
			xAutoTrasfDaAccumulo = false;
		}

		if (ferma_accumulo || s().xGasInUso || (s().wTemperaturaACS < 45) || ((s().wTemperaturaACS < 69) && (s().wTemperaturaAccumulo > s().wTemperaturaACS-2))) {
			/* stop trasf HPSU->Accumulo (salvataggio energia)*/
			xAutoTrasfVersoAccumulo = false;
		}

		/* se trasf Accumulo->HPSU attivo -> no caldaia */
		if (xAutoTrasfDaAccumulo)
			acs_attiva = false;

		if (s().xSetManuale) {
			s().xTrasfDaAccumuloInCorso(O_CTRL) = s().xTrasfDaAccumulo;
			s().xTrasfVersoAccumuloInCorso(O_CTRL) = s().xTrasfVersoAccumulo;
		} else {
			s().xTrasfDaAccumuloInCorso(O_CTRL) = xAutoTrasfDaAccumulo;
			s().xTrasfVersoAccumuloInCorso(O_CTRL) = xAutoTrasfVersoAccumulo;
		}

		bool start_pompa_accumulo = (s().xTrasfDaAccumuloInCorso || s().xTrasfVersoAccumuloInCorso);
		static DelayRiseTimer tDurataPompaAccumulo;
		bool max_durata_pompa_accumulo = tDurataPompaAccumulo.update(DELAY_MIN(120), start_pompa_accumulo && s().xSetManuale);
		HW.Accumulo.xStartPompa->setValue(start_pompa_accumulo && !max_durata_pompa_accumulo);
		if (max_durata_pompa_accumulo) {
			s().xTrasfDaAccumuloInCorso(O_CTRL) = false;
			s().xTrasfVersoAccumuloInCorso(O_CTRL) = false;
		}

		if (acs_attiva && !s().xPompaCaloreRiscInUso && !s().xDisabilitaGas) {
			/* auto mode */
			if (s().wTemperaturaACS < 50)
				xAutoGas = true;
		else if (s().wTemperaturaACS > 55)
			xAutoGas = false;
		} else {
			xAutoGas = false;
		}

		if (s().xSetManuale)
			s().xGasInUso(O_CTRL) = s().xUsaGas;
		else
			s().xGasInUso(O_CTRL) = xAutoGas;

		HW.Gas.xAlimenta->setValue(s().xGasInUso);
		static DelayRiseTimer tStartGas;
		HW.Gas.xStartCaldaia->setValue(tStartGas.update(DELAY_SEC(10), s().xGasInUso));
		static DelayFallTimer tStartPompa;
		HW.Gas.xStartPompa->setValue(tStartPompa.update(DELAY_SEC(60), HW.Gas.xStartCaldaia->getValue()));

		s().fieldLock.unlock();

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
