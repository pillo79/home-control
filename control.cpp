#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"
#include "powercalc.h"

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
	:  wTemperaturaACS	("°C", MAX_PTS)
	,  wTemperaturaBoiler	("°C", MAX_PTS)
	,  wTemperaturaAccumulo	("°C", MAX_PTS)
	,  wTemperaturaPannelli	("°C", MAX_PTS)

	,  wPotProdotta		("W", MAX_PTS)
	,  wPotConsumata	("W", MAX_PTS)
	,  wPotResistenze	("W", MAX_PTS)
	,  wEnergProdotta	("kWh", MAX_PTS)
	,  wEnergConsumata	("kWh", MAX_PTS)
	,  wEnergPassivo	("kWh", MAX_PTS)

	,  wTempGiorno		("°C", MAX_PTS)
	,  wUmidGiorno		("%", MAX_PTS)
	,  wTempNotte		("°C", MAX_PTS)
	,  wUmidNotte		("%", MAX_PTS)
	,  wTempSoffitta	("°C", MAX_PTS)
	,  wUmidSoffitta	("%", MAX_PTS)
	,  wTempEsterno		("°C", MAX_PTS)
	,  wUmidEsterno		("%", MAX_PTS)
{
	wVelFanCoil = 20;
	xModoRiscaldamento = true;

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

void ControlThread::setPowerLevel(int level)
{
	for (int i=0; i<5; ++i)
		HW.PompaCalore.xConfigResistenze[i]->setValue(POWER_LEVEL[level].io_map[i]);

	xResistenzeInUso = (level > 0);
}

void ControlThread::run()
{
	ModbusDevice::openSerial("/dev/ttyUSB0", 38400, 'N', 8, 1);
	InitHardware();

	QTime lastTime = QTime::currentTime();
	int lastSecs = lastTime.hour()*3600 + lastTime.minute()*60 + lastTime.second();

	bool xAutoCaldaia = false;
	bool xAutoPompaCaloreRisc = false;
	bool xAutoPompaCaloreCond = false;
	bool xAutoTrasfDaAccumulo = false;
	bool xAutoTrasfVersoAccumulo = false;
	bool xCaricoAccumuloAttivo = false;

	bool xRichiestaRestart = false;

	int PowerLevel = 0;
	int NextPowerLevel = -1;

	while(1) {

		static PowerCalc pcProdotta;
		static PowerCalc pcConsumata;
		static PowerCalc pcResistenze;

		QTime rd_time = QTime::currentTime();
		ReadHardwareInputs();
		int rd_ms = rd_time.elapsed();

		mFields.lock();

		wCommErrorMask = GetCommErrorMask();

		// impulso reset PLC
		static DelayRiseTimer tRichiestaRestart;
		if (wCommErrorMask) {
			pcProdotta.restart();
			pcConsumata.restart();
			pcResistenze.restart();
			if (!xRichiestaRestart) {
				xRichiestaRestart = true;
				dtLastResetPLC = QDateTime::currentDateTime();
				++wResetPLCs;
			}
		}
		bool xRitardoRichiestaRestart = tRichiestaRestart.update(DELAY_SEC(1), xRichiestaRestart);
		HW.xResetPLC->setValue(xRichiestaRestart && !xRitardoRichiestaRestart);
		if (xRitardoRichiestaRestart && !wCommErrorMask)
			xRichiestaRestart = false;

		wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue() / 10.0;
		wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue() / 10.0;
		wTemperaturaAccumulo = HW.Accumulo.wTemperatura->getValue() / 10.0;
		wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue() / 10.0;

		wTempSoffitta = HW.Ambiente.wTemperaturaSoffitta->getValue() / 10.0;
		wUmidSoffitta = HW.Ambiente.wUmiditaSoffitta->getValue() / 10.0;
		wTempEsterno =  HW.Ambiente.wTemperaturaEsterna->getValue() / 10.0;

		Timer::tick();
		QTime now = QTime::currentTime();
		int nowSecs = now.hour()*3600 + now.minute()*60 + now.second();

		if ((nowSecs != lastSecs) && ((nowSecs % SAMPLE_PERIOD_SECS) == 0) && !xRichiestaRestart) {
			pcProdotta.addSample(HW.Pannelli.wPotenzaProdotta->getValue());
			wPotProdotta = pcProdotta.getCurrentPower();
			wEnergProdotta = pcProdotta.getCurrentEnergy() / 1000.0;
			pcConsumata.addSample(HW.Pannelli.wPotenzaConsumata->getValue());
			wPotConsumata = pcConsumata.getCurrentPower();
			wEnergConsumata = pcConsumata.getCurrentEnergy() / 1000.0;
			pcResistenze.addSample(HW.Pannelli.wPotenzaResistenze->getValue());
			wPotResistenze = pcResistenze.getCurrentPower25();
			printf("\n");
		}

		if (now.minute() != lastTime.minute()) {
			int prod = pcProdotta.getDeltaSteps();
			int cons = pcConsumata.getDeltaSteps();
			if (prod > cons)
				tTempoAttivo = tTempoAttivo.addSecs(60);
			else
				wEnergPassivo = wEnergPassivo + (cons-prod) / 1000.0;

			if ((wPotProdotta < 1500) || (wPotConsumata > wPotProdotta) || (wTemperaturaBoiler > 50))
				xAutoPompaCaloreRisc = false;
			else if (wPotProdotta > 2500)
				xAutoPompaCaloreRisc = true;

			if ((now.minute() % 3) == 0) {
				int power_budget = pcProdotta.getCurrentPower25() + pcResistenze.getCurrentPower25() - pcConsumata.getCurrentPower25();
				int next_level;

				// never allocate more power than currently produced
				if (power_budget > pcProdotta.getCurrentPower25())
					power_budget = pcProdotta.getCurrentPower25();

				for (next_level=POWER_LEVELS-1; next_level>=0; --next_level) {
					if (power_budget > POWER_LEVEL[next_level].min_budget)
						break;
				}

				if (next_level != PowerLevel) {
					NextPowerLevel = next_level;
					setPowerLevel(0);
					printf("%4i +%4i[%i] -%4i = %4i => SWITCH TO %4i[%i]\n",
						pcProdotta.getCurrentPower25(), pcResistenze.getCurrentPower25(), PowerLevel, pcConsumata.getCurrentPower25(),
						power_budget, POWER_LEVEL[NextPowerLevel].power, NextPowerLevel);
				} else {
					printf("%4i +%4i[%i] -%4i = %4i => NO CHANGE\n",
						pcProdotta.getCurrentPower25(), pcResistenze.getCurrentPower25(), PowerLevel, pcConsumata.getCurrentPower25(),
						power_budget);
				}

				// store and advance all tracked values
				wTemperaturaACS.step();
				wTemperaturaBoiler.step();
				wTemperaturaAccumulo.step();
				wTemperaturaPannelli.step();

				wPotProdotta.step();
				wPotConsumata.step();
				wPotResistenze.step();
				wEnergProdotta.step();
				wEnergConsumata.step();
				wEnergPassivo.step();

				wTempGiorno.step();
				wUmidGiorno.step();
				wTempNotte.step();
				wUmidNotte.step();
				wTempSoffitta.step();
				wUmidSoffitta.step();
				wTempEsterno.step();
				wUmidEsterno.step();
			}
		}

		if (now.hour() != lastTime.hour()) {
			switch (now.hour()) {
			case 0:
				// NEXT DAY!
				pcProdotta.resetTotals();
				pcConsumata.resetTotals();
				tTempoAttivo.setHMS(0,0,0);
				wEnergPassivo = 0.0;
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

		bool zone_accese = xAttivaZonaNotte || xAttivaZonaGiorno || xAttivaZonaSoffitta;
		bool impianto_acceso = zone_accese || xAttivaFanCoil;
		bool risc_manuale_solo_hp = xSetManuale && xModoRiscaldamento && xUsaPompaCalore && !xUsaCaldaia;
		HW.Riscaldamento.xChiudiValvola->setValue(!zone_accese);

		static DelayRiseTimer tStartPompe;
		bool start_pompe = tStartPompe.update(DELAY_SEC(20), zone_accese);
		HW.Riscaldamento.xStartPompaGiorno->setValue(start_pompe && xAttivaZonaGiorno);
		HW.Riscaldamento.xStartPompaNotte->setValue(start_pompe && xAttivaZonaNotte);
		HW.Riscaldamento.xStartPompaSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilStanzaSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilBagnoSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartPompaCircuito->setValue(start_pompe);

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
		HW.PompaCalore.xForza3VieApri->setValue(impianto_acceso && xAutoPompaCaloreCond);
		HW.PompaCalore.xForza3VieChiudi->setValue(impianto_acceso && !xAutoPompaCaloreCond);

		// *** 3UV1 (movimento lungo, quella che si ferma a 3/4)
		// risc o ACS:			3/4 (non alimentata + burst)
		// risc solo con pompa calore:	chiusa (non alimentata)
		// condiz:			chiusa (non alimentata)
		static DelayRiseTimer tResetManValvole;
		bool reset_man_finito = tResetManValvole.update(DELAY_SEC(8), impianto_acceso);
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

		bool muovi_serranda = xApriCucina || xChiudiCucina;
		HW.FanCoilCorridoio.xMuoviSerrandaCucina->setValue(muovi_serranda);
		HW.FanCoilCorridoio.xApriSerrandaCucina->setValue(xApriCucina);
		static DelayRiseTimer tStopSerrandaCucina;
		bool stop_serranda = tStopSerrandaCucina.update(xChiudiCucina? DELAY_SEC(30) : DELAY_SEC(13), muovi_serranda);
		if (stop_serranda)  {
			xApriCucina = xChiudiCucina = false;
		}

		if (xSetManuale) {
			xPompaCaloreRiscInUso = xUsaPompaCalore && xModoRiscaldamento;
			xPompaCaloreCondInUso = xUsaPompaCalore && !xModoRiscaldamento;
		} else {
			xPompaCaloreRiscInUso = xAutoPompaCaloreRisc;
			xPompaCaloreCondInUso = xAutoPompaCaloreCond;
		}

		HW.PompaCalore.xStopPompaCalore->setValue(!(xPompaCaloreRiscInUso || xPompaCaloreCondInUso));
		HW.PompaCalore.xRichiestaCaldo->setValue(xPompaCaloreRiscInUso);
		HW.PompaCalore.xRichiestaFreddo->setValue(xPompaCaloreCondInUso);

		bool acs_attiva = false;
		if (!xPompaCaloreRiscInUso && (wTemperaturaACS < 55)) {
			/* caldaia auto */
			acs_attiva |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			acs_attiva |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		}
		if (xModoRiscaldamento)
			acs_attiva |= zone_accese;

		// condizioni HPSU->accumulo ("salvataggio energia")
		// abilitato quando tBoiler > 60C
		// disabilitato ogni giorno alle 18
		if ((now<QTime(10,0)) || (now>QTime(18,0)))
			xCaricoAccumuloAttivo = false;
		else if (wTemperaturaBoiler > 60)
			xCaricoAccumuloAttivo = true;

		if (acs_attiva && !xDisabilitaAccumulo) {
			/* trasf Accumulo->HPSU (uso energia) */
			if ((wTemperaturaAccumulo > 55) && (wTemperaturaAccumulo > wTemperaturaACS+10.0) && !xCaldaiaInUso) {
				xAutoTrasfDaAccumulo = true;
			} else if (xCaldaiaInUso || (wTemperaturaAccumulo < 50) || (wTemperaturaAccumulo < wTemperaturaACS+5)) {
				xAutoTrasfDaAccumulo = false;
			}

			/* trasf Accumulo->HPSU attivo -> no caldaia */
			if (xAutoTrasfDaAccumulo)
				acs_attiva = false;

			/* reset condizione opposta */
			xAutoTrasfVersoAccumulo = false;
		} else if (xCaricoAccumuloAttivo && !xDisabilitaAccumulo) {
			/* trasf HPSU->Accumulo (salvataggio energia)*/
			if ((wTemperaturaACS > 50) && ((wTemperaturaAccumulo < wTemperaturaACS-6) || (wTemperaturaACS > 70)) && !xCaldaiaInUso) {
				xAutoTrasfVersoAccumulo = true;
			} else if (xCaldaiaInUso || (wTemperaturaACS < 40) || (wTemperaturaAccumulo > wTemperaturaACS-3)) {
				xAutoTrasfVersoAccumulo = false;
			}

			/* reset condizione opposta */
			xAutoTrasfDaAccumulo = false;
		} else {
			/* trasf HPSU<->Accumulo non permesso */
			xAutoTrasfDaAccumulo = false;
			xAutoTrasfVersoAccumulo = false;
		}

		if (xSetManuale) {
			xTrasfDaAccumuloInCorso = xTrasfDaAccumulo;
			xTrasfVersoAccumuloInCorso = xTrasfVersoAccumulo;
		} else {
			xTrasfDaAccumuloInCorso = xAutoTrasfDaAccumulo;
			xTrasfVersoAccumuloInCorso = xAutoTrasfVersoAccumulo;
		}

		static DelayFallTimer tCambioDirAccumulo;
		static bool ultimo_trasf_da_accumulo = false;
		bool cambio_dir_accumulo = tCambioDirAccumulo.update(DELAY_MIN(5), (xTrasfDaAccumuloInCorso != ultimo_trasf_da_accumulo));
		ultimo_trasf_da_accumulo = xTrasfDaAccumuloInCorso;

		HW.Accumulo.xAcquaDaAccumulo->setValue(xTrasfDaAccumuloInCorso);

		bool ok_start_pompa_accumulo = (xTrasfDaAccumuloInCorso || xTrasfVersoAccumuloInCorso) && !cambio_dir_accumulo;
		static DelayRiseTimer tDurataPompaAccumulo;
		bool max_durata_pompa_accumulo = tDurataPompaAccumulo.update(DELAY_MIN(120), ok_start_pompa_accumulo && xSetManuale);
		HW.Accumulo.xStartPompa->setValue(ok_start_pompa_accumulo && !max_durata_pompa_accumulo);
		if (max_durata_pompa_accumulo) {
			xTrasfDaAccumuloInCorso = false;
			xTrasfVersoAccumuloInCorso = false;
		}

		if (acs_attiva && !xDisabilitaCaldaia) {
			/* auto mode */
			if (wTemperaturaACS < 50)
				xAutoCaldaia = true;
			else if (wTemperaturaACS > 55)
				xAutoCaldaia = false;
		} else {
			xAutoCaldaia = false;
		}

		if (xSetManuale)
			xCaldaiaInUso = xUsaCaldaia;
		else
			xCaldaiaInUso = xAutoCaldaia;

		HW.Caldaia.xAlimenta->setValue(xCaldaiaInUso);
		static DelayRiseTimer tStartCaldaia;
		HW.Caldaia.xStartCaldaia->setValue(tStartCaldaia.update(DELAY_SEC(10), xCaldaiaInUso));
		static DelayFallTimer tStartPompa;
		HW.Caldaia.xStartPompa->setValue(tStartPompa.update(DELAY_SEC(60), HW.Caldaia.xStartCaldaia->getValue()));

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
