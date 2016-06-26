#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"
#include "powercalc.h"

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
	wVelFanCoil = 20;

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

	wPotResistenze = POWER_LEVEL[level].power;
	xResistenzeInUso = (level > 0);
}

void ControlThread::run()
{
	ModbusDevice::openSerial("/dev/ttyUSB0", 38400, 'N', 8, 1);
	InitHardware();

	QTime lastTime = QTime::currentTime();
	int lastSecs = lastTime.hour()*3600 + lastTime.minute()*60 + lastTime.second();

	bool xAutoCaldaia = false;
	bool xAutoPompaCalore = false;
	bool xAutoTrasfDaAccumulo = false;
	bool xAutoTrasfVersoAccumulo = false;
	bool xCaricoAccumuloAttivo = false;

	int PowerLevel = 0;
	int NextPowerLevel = -1;

	while(1) {

		static PowerCalc pcProdotta;
		static PowerCalc pcConsumata;

		QTime rd_time = QTime::currentTime();
		ReadHardwareInputs();
		int rd_ms = rd_time.elapsed();

		mFields.lock();

		wCommErrorMask = GetCommErrorMask();

		wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue();
		wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue();
		wTemperaturaAccumulo = HW.Accumulo.wTemperatura->getValue();
		wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue();

		wTempSoffitta = HW.Ambiente.wTemperaturaSoffitta->getValue();
		wUmidSoffitta = HW.Ambiente.wUmiditaSoffitta->getValue();
		wTempEsterno =  HW.Ambiente.wTemperaturaEsterna->getValue();

		Timer::tick();
		QTime now = QTime::currentTime();
		int nowSecs = now.hour()*3600 + now.minute()*60 + now.second();

		if ((nowSecs != lastSecs) && ((nowSecs % SAMPLE_PERIOD_SECS) == 0)) {
			pcProdotta.addSample(HW.Pannelli.wPotenzaProdotta->getValue());
			wPotProdotta = pcProdotta.getCurrentPower();
			wEnergProdotta = pcProdotta.getCurrentEnergy();
			pcConsumata.addSample(HW.Pannelli.wPotenzaConsumata->getValue());
			wPotConsumata = pcConsumata.getCurrentPower();
			wEnergConsumata = pcConsumata.getCurrentEnergy();
			printf("\n");
		}

		if (now.minute() != lastTime.minute()) {
			int prod = pcProdotta.getDeltaSteps();
			int cons = pcConsumata.getDeltaSteps();
			if (prod > cons)
				tTempoAttivo = tTempoAttivo.addSecs(60);
			else
				wEnergPassivo += cons-prod;

			if ((wPotProdotta < 1500) || (wPotConsumata > wPotProdotta))
				xAutoPompaCalore = false;
			else if (wPotProdotta > 2500)
				xAutoPompaCalore = true;

			if ((now.minute() % 3) == 0) {
				int power_budget = pcProdotta.getCurrentPower25() + POWER_LEVEL[PowerLevel].power - pcConsumata.getCurrentPower25();
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
						pcProdotta.getCurrentPower25(), POWER_LEVEL[PowerLevel].power, PowerLevel, pcConsumata.getCurrentPower25(),
						power_budget, POWER_LEVEL[NextPowerLevel].power, NextPowerLevel);
				} else {
					printf("%4i +%4i[%i] -%4i = %4i => NO CHANGE\n",
						pcProdotta.getCurrentPower25(), POWER_LEVEL[PowerLevel].power, PowerLevel, pcConsumata.getCurrentPower25(),
						power_budget);
				}
			}
		}

		if (now.hour() != lastTime.hour()) {
			switch (now.hour()) {
			case 0:
				// NEXT DAY!
				pcProdotta.resetTotals();
				pcConsumata.resetTotals();
				tTempoAttivo.setHMS(0,0,0);
				wEnergPassivo = 0;
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

		bool zone_accese = xAttivaZonaNotte || xAttivaZonaGiorno || xAttivaZonaSoffitta;
		HW.Riscaldamento.xChiudiValvola->setValue(!zone_accese);
		static DelayRiseTimer tStartPompe;
		bool start_pompe = tStartPompe.update(DELAY_SEC(20), zone_accese);
		HW.Riscaldamento.xStartPompaGiorno->setValue(start_pompe && xAttivaZonaGiorno);
		HW.Riscaldamento.xStartPompaNotte->setValue(start_pompe && xAttivaZonaNotte);
		HW.Riscaldamento.xStartPompaSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilStanzaSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);
		HW.Riscaldamento.xStartFanCoilBagnoSoffitta->setValue(start_pompe && xAttivaZonaSoffitta);

		// setup valvole pompa calore
		static DelayRiseTimer tResetManValvole;
		bool reset_man_finito = tResetManValvole.update(DELAY_SEC(8), zone_accese);
		static DelayRiseTimer tSetPosValvolaUV1;
		bool set_pos_uv1_finito = tSetPosValvolaUV1.update(DELAY_MSEC(1900), reset_man_finito);
		HW.PompaCalore.xForzaValvole->setValue(zone_accese);
		HW.PompaCalore.xForza3VieApri->setValue(zone_accese && false);
		HW.PompaCalore.xForza3VieChiudi->setValue(zone_accese && true);
		HW.PompaCalore.xForzaRiscApri->setValue(zone_accese && !reset_man_finito);
		HW.PompaCalore.xForzaRiscFerma->setValue(set_pos_uv1_finito);

		static DelayRiseTimer tNuovoLivelloRes;
		if ((wTemperaturaACS > 800) || (wTemperaturaBoiler > 800) || (wPotConsumata < 100)) {
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

		HW.FanCoilCorridoio.xChiudiValvola->setValue(!xAttivaFanCoil);
		static DelayRiseTimer tStartFanCoil;
		HW.FanCoilCorridoio.xStartVentilatore->setValue(tStartFanCoil.update(DELAY_SEC(30), xAttivaFanCoil));
		HW.FanCoilCorridoio.wLivelloVentilatore->setValue(wVelFanCoil*200);

		bool muovi_serranda = xApriCucina || xChiudiCucina;
		HW.FanCoilCorridoio.xMuoviSerrandaCucina->setValue(muovi_serranda);
		HW.FanCoilCorridoio.xApriSerrandaCucina->setValue(xApriCucina);
		static DelayRiseTimer tStopSerrandaCucina;
		bool stop_serranda = tStopSerrandaCucina.update(xChiudiCucina? DELAY_SEC(30) : DELAY_SEC(13), muovi_serranda);
		if (stop_serranda)  {
			xApriCucina = xChiudiCucina = false;
		}

		if (xSetManuale)
			xPompaCaloreInUso = xUsaPompaCalore;
		else
			xPompaCaloreInUso = xAutoPompaCalore;

		HW.PompaCalore.xStopPompaCalore->setValue(!xPompaCaloreInUso);
		HW.PompaCalore.xRichiestaCaldo->setValue(zone_accese || xAttivaFanCoil);

		bool acs_attiva = false;
		if (!xPompaCaloreInUso && (wTemperaturaACS < 550)) {
			/* caldaia auto */
			acs_attiva |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			acs_attiva |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		}
		acs_attiva |= zone_accese;

		// condizioni HPSU->accumulo ("salvataggio energia")
		// abilitato ogni giorno fino alle 18
		if ((now<QTime(10,0)) || (now>QTime(18,0)))
			xCaricoAccumuloAttivo = false;
		else if (wTemperaturaBoiler > 600)
			xCaricoAccumuloAttivo = true;

		if (acs_attiva) {
			/* trasf Accumulo->HPSU (uso energia) */
			if ((wTemperaturaAccumulo > 550) && (wTemperaturaAccumulo > wTemperaturaACS+100) && !xCaldaiaInUso) {
				xAutoTrasfDaAccumulo = true;
			} else if (xCaldaiaInUso || (wTemperaturaAccumulo < 500) || (wTemperaturaAccumulo < wTemperaturaACS+50)) {
				xAutoTrasfDaAccumulo = false;
			}

			/* trasf Accumulo->HPSU attivo -> no caldaia */
			if (xAutoTrasfDaAccumulo)
				acs_attiva = false;

			/* reset condizione opposta */
			xAutoTrasfVersoAccumulo = false;
		} else if (xCaricoAccumuloAttivo) {
			/* trasf HPSU->Accumulo (salvataggio energia)*/
			if ((wTemperaturaACS > 500) && (wTemperaturaAccumulo < wTemperaturaACS-60) && !xCaldaiaInUso) {
				xAutoTrasfVersoAccumulo = true;
			} else if (xCaldaiaInUso || (wTemperaturaACS < 400) || (wTemperaturaAccumulo > wTemperaturaACS-30)) {
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

		if (acs_attiva) {
			/* auto mode */
			if (wTemperaturaACS < 500)
				xAutoCaldaia = true;
			else if (wTemperaturaACS > 550)
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
