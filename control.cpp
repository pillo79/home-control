#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"
#include "powercalc.h"

#include <QTime>

ControlThread::ControlThread()
{
	wVelFanCoil = 20;
	xUsaPompaCalore = true;

	start();
}

ControlThread::~ControlThread()
{

}

void ControlThread::run()
{
	ModbusDevice::openSerial("/dev/ttyUSB0", 38400, 'N', 8, 1);
	InitHardware();

	while(1) {
		ReadHardwareInputs();

		mFields.lock();

		Timer::tick();

		wTemperaturaACS = HW.PompaCalore.wTemperaturaACS->getValue();
		wTemperaturaBoiler = HW.PompaCalore.wTemperaturaBoiler->getValue();
		wTemperaturaAccumuli = HW.Accumuli.wTemperatura->getValue();
		wTemperaturaPannelli = HW.PompaCalore.wTemperaturaPannelli->getValue();

		static PeriodicTimer tImpiantoElettrico;
		static PowerCalc pcProdotta;
		static PowerCalc pcConsumata;
		if (tImpiantoElettrico.update(DELAY_SEC(3), true)) {
			pcProdotta.addSample(HW.Pannelli.wPotenzaProdotta->getValue());
			wPotProdotta = pcProdotta.getCurrentPower();
			pcConsumata.addSample(HW.Pannelli.wPotenzaConsumata->getValue());
			wPotConsumata = pcConsumata.getCurrentPower();
		}

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
		HW.PompaCalore.xRichiestaCaldo->setValue(risc_acceso || xFanCoil);

		bool zona_attiva = false;
		if (!xUsaPompaCalore) {
			/* caldaia auto */
			zona_attiva |= ((QTime::currentTime()>QTime(6,0)) && (QTime::currentTime()<QTime(8,30)));
			zona_attiva |= ((QTime::currentTime()>QTime(11,0)) && (QTime::currentTime()<QTime(15,0)));
			zona_attiva |= ((QTime::currentTime()>QTime(18,0)) && (QTime::currentTime()<QTime(21,0)));
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
	}
}

ControlThread &control()
{
	static ControlThread theControl;
	return theControl;
}
