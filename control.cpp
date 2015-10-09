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
	xUsaPompaCalore = true;

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

	while(1) {

		static PowerCalc pcProdotta;
		static PowerCalc pcConsumata;

		ReadHardwareInputs();

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
		}

		if (now.minute() != lastTime.minute())
		{
			int prod = pcProdotta.getDeltaSteps();
			int cons = pcConsumata.getDeltaSteps();
			if (prod > cons)
				tTempoAttivo.addSecs(60);
		}

		if (now.hour() != lastTime.hour()) {
			switch (now.hour()) {
			case 0:
				// NEXT DAY!
				pcProdotta.resetTotals();
				pcConsumata.resetTotals();
				tTempoAttivo.setHMS(0,0,0);
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

		mFields.lock();

		wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue();
		wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue();
		wTemperaturaAccumuli = HW.Accumuli.wTemperatura->getValue();
		wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue();

		wTempSoffitta = HW.Ambiente.wTemperaturaSoffitta->getValue();
		wUmidSoffitta = HW.Ambiente.wUmiditaSoffitta->getValue();

		bool risc_acceso = xRiscaldaNotte || xRiscaldaGiorno || xRiscaldaSoffitta;
		HW.Riscaldamento.xChiudiValvola->setValue(!risc_acceso);
		static DelayRiseTimer tStartPompe;
		bool start_pompe = tStartPompe.update(DELAY_SEC(20), risc_acceso);
		HW.Riscaldamento.xStartPompaGiorno->setValue(start_pompe && xRiscaldaGiorno);
		HW.Riscaldamento.xStartPompaNotte->setValue(start_pompe && xRiscaldaNotte);
		HW.Riscaldamento.xStartPompaSoffitta->setValue(start_pompe && xRiscaldaSoffitta);
		HW.Riscaldamento.xStartFanCoilStanzaSoffitta->setValue(start_pompe && xRiscaldaSoffitta);
		HW.Riscaldamento.xStartFanCoilBagnoSoffitta->setValue(start_pompe && xRiscaldaSoffitta);

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

		static DelayRiseTimer tStopForzaPompaCalore;
		bool stop_forza_pompa_calore = tStopForzaPompaCalore.update(DELAY_MIN(5), risc_acceso || xFanCoil);
		if ((risc_acceso || xFanCoil) && !stop_forza_pompa_calore)
			xPompaCaloreInUso = true;
		else
			xPompaCaloreInUso = xUsaPompaCalore;
		HW.PompaCalore.xStopPompaCalore->setValue(!xPompaCaloreInUso);
//		HW.PompaCalore.xRichiestaCaldo->setValue(risc_acceso || xFanCoil);

		bool zona_attiva = false;
		if (!xUsaPompaCalore) {
			/* caldaia auto */
			zona_attiva |= ((now>QTime(6,0)) && (now<QTime(8,30)));
			zona_attiva |= ((now>QTime(11,0)) && (now<QTime(15,0)));
			zona_attiva |= ((now>QTime(18,0)) && (now<QTime(21,0)));
		}
		zona_attiva |= stop_forza_pompa_calore;
		if (zona_attiva) {
			/* auto mode: caldaia = richiesta acs | bottone */
			if (wTemperaturaACS < 450)
				xCaldaiaInUso = true;
			else if (wTemperaturaACS > 500)
				xCaldaiaInUso = false;
			xCaldaiaInUso |= xUsaCaldaia;
		} else {
			/* man mode: caldaia = bottone */
			xCaldaiaInUso = xUsaCaldaia;
		}

		HW.Caldaia.xAlimenta->setValue(xCaldaiaInUso);
		static DelayRiseTimer tStartCaldaia;
		HW.Caldaia.xStartCaldaia->setValue(tStartCaldaia.update(DELAY_SEC(10), xCaldaiaInUso));
		static DelayFallTimer tStartPompa;
		HW.Caldaia.xStartPompa->setValue(tStartPompa.update(DELAY_SEC(60), HW.Caldaia.xStartCaldaia->getValue()));

		HW.Accumuli.xApriValvola->setValue(xTrasfAccumulo);
		static DelayRiseTimer tStartPompaAccumulo;
		HW.Accumuli.xStartPompa->setValue(tStartPompaAccumulo.update(DELAY_SEC(30), xTrasfAccumulo));

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
