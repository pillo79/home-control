#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"
#include "powercalc.h"

#include <QTime>
#include <stdio.h>

ControlThread::ControlThread()
{
	wVelFanCoil = 20;

	start();
}

ControlThread::~ControlThread()
{

}

void echo(const char *what, const char *where)
{
	FILE *f = fopen(where, "w");
	fprintf(f, "%s", what);
	fclose(f);
}

void ControlThread::run()
{
	ModbusDevice::openSerial("/dev/ttyUSB0", 38400, 'N', 8, 1);
	InitHardware();

	QTime lastTime = QTime::currentTime();
	int lastSecs = lastTime.hour()*3600 + lastTime.minute()*60 + lastTime.second();

	bool xAutoResistenze = false;
	bool xAutoCaldaia = false;
	bool xAutoPompaCalore = false;
	bool xAutoTrasfAccumulo = false;

	while(1) {

		static PowerCalc pcProdotta;
		static PowerCalc pcConsumata;

		ReadHardwareInputs();

		mFields.lock();

		wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue();
		wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue();
		wTemperaturaAccumulo = HW.Accumulo.wTemperatura->getValue();
		wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue();

		wTempSoffitta = HW.Ambiente.wTemperaturaSoffitta->getValue();
		wUmidSoffitta = HW.Ambiente.wUmiditaSoffitta->getValue();

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

			if (wPotProdotta > 2000)
				xAutoPompaCalore = true;
			else if (wPotProdotta < 1000)
				xAutoPompaCalore = false;

			if ((wPotProdotta-wPotConsumata > 2000) && (wTemperaturaACS > 550))
				xAutoResistenze = true;
		}

		if (wPotProdotta-wPotConsumata < 500)
			xAutoResistenze = false;

		if (now.hour() != lastTime.hour()) {
			switch (now.hour()) {
			case 0:
				// NEXT DAY!
				pcProdotta.resetTotals();
				pcConsumata.resetTotals();
				tTempoAttivo.setHMS(0,0,0);
				wEnergPassivo = 0;
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

		bool risc_acceso = xRiscaldaNotte || xRiscaldaGiorno || xRiscaldaSoffitta;
		HW.Riscaldamento.xChiudiValvola->setValue(!risc_acceso);
		static DelayRiseTimer tStartPompe;
		bool start_pompe = tStartPompe.update(DELAY_SEC(20), risc_acceso);
		HW.Riscaldamento.xStartPompaGiorno->setValue(start_pompe && xRiscaldaGiorno);
		HW.Riscaldamento.xStartPompaNotte->setValue(start_pompe && xRiscaldaNotte);
		HW.Riscaldamento.xStartPompaSoffitta->setValue(start_pompe && xRiscaldaSoffitta);
		HW.Riscaldamento.xStartFanCoilStanzaSoffitta->setValue(start_pompe && xRiscaldaSoffitta);
		HW.Riscaldamento.xStartFanCoilBagnoSoffitta->setValue(start_pompe && xRiscaldaSoffitta);

		// setup valvole pompa calore
		static DelayRiseTimer tResetManValvole;
		bool reset_man_finito = tResetManValvole.update(DELAY_SEC(8), risc_acceso);
		static DelayRiseTimer tSetPosValvolaUV1;
		bool set_pos_uv1_finito = tSetPosValvolaUV1.update(DELAY_MSEC(1900), reset_man_finito);
		HW.PompaCalore.xForzaValvole->setValue(risc_acceso);
		HW.PompaCalore.xForza3VieApri->setValue(risc_acceso && false);
		HW.PompaCalore.xForza3VieChiudi->setValue(risc_acceso && true);
		HW.PompaCalore.xForzaRiscApri->setValue(risc_acceso && !reset_man_finito);
		HW.PompaCalore.xForzaRiscFerma->setValue(set_pos_uv1_finito);

		if (xSetManuale)
			xResistenzeInUso = xUsaResistenze;
		else
			xResistenzeInUso = xAutoResistenze;
		xResistenzeInUso = xResistenzeInUso && (wTemperaturaACS < 800) && (wTemperaturaBoiler < 800);
		HW.PompaCalore.xInserResistenze->setValue(xResistenzeInUso);

		HW.FanCoilCorridoio.xChiudiValvola->setValue(!xFanCoil);
		static DelayRiseTimer tStartFanCoil;
		HW.FanCoilCorridoio.xStartVentilatore->setValue(tStartFanCoil.update(DELAY_SEC(30), xFanCoil));
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
		HW.PompaCalore.xRichiestaCaldo->setValue(risc_acceso || xFanCoil);

		bool zona_attiva = false;
		if (!xPompaCaloreInUso && (wTemperaturaACS < 550)) {
			/* caldaia auto */
			zona_attiva |= ((now>QTime(11,0)) && (now<QTime(14,0)));
			zona_attiva |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		}
		zona_attiva |= risc_acceso;

		if (zona_attiva) {
			if ((wTemperaturaAccumulo > 600) && (wTemperaturaAccumulo > wTemperaturaBoiler+150) && !xCaldaiaInUso) {
				/* trasf Accumulo->HPSU */
				xAutoTrasfAccumulo = true;
			} else if (xCaldaiaInUso || (wTemperaturaAccumulo < 500) || (wTemperaturaAccumulo < wTemperaturaBoiler+50)) {
				xAutoTrasfAccumulo = false;
			}

			if (xAutoTrasfAccumulo)
				zona_attiva = false;
		} else {
			if ((wTemperaturaBoiler > 500) && (wTemperaturaAccumulo < wTemperaturaBoiler-150) && !xCaldaiaInUso) {
				/* trasf HPSU->Accumulo */
				xAutoTrasfAccumulo = true;
			} else if (xCaldaiaInUso || (wTemperaturaBoiler < 400) || (wTemperaturaAccumulo > wTemperaturaBoiler-50)) {
				xAutoTrasfAccumulo = false;
			}
		}

		if (xSetManuale)
			xTrasfAccumuloInCorso = xTrasfAccumulo;
		else
			xTrasfAccumuloInCorso = xAutoTrasfAccumulo;

		HW.Accumulo.xApriValvola->setValue(xTrasfAccumuloInCorso);

		static DelayRiseTimer tStartPompaAccumulo;
		bool ok_start_pompa_accumulo = tStartPompaAccumulo.update(DELAY_MIN(2), xTrasfAccumuloInCorso);
		static DelayRiseTimer tDurataPompaAccumulo;
		bool max_durata_pompa_accumulo = tDurataPompaAccumulo.update(DELAY_MIN(60), ok_start_pompa_accumulo);
		if (!xSetManuale)
			max_durata_pompa_accumulo = false; // non interrompere in automatico
		HW.Accumulo.xStartPompa->setValue(ok_start_pompa_accumulo && !max_durata_pompa_accumulo);
		if (max_durata_pompa_accumulo)
			xTrasfAccumuloInCorso = false;

		if (zona_attiva) {
			/* auto mode */
			if (wTemperaturaACS < 450)
				xAutoCaldaia = true;
			else if (wTemperaturaACS > 500)
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
		WriteHardwareOutputs();
		lastTime = now;
		lastSecs = nowSecs;
	}
}

ControlThread &control()
{
	static ControlThread theControl;
	return theControl;
}
