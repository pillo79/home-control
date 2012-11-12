#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc_lib.h"

#include <QTime>

ControlThread::ControlThread()
{
	start();
	wVelFanCoil = 50;
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
		HW.FanCoilCorridoio.xChiudiSerrandaCucina->setValue(xChiudiCucina);
		static DelayRiseTimer tStopSerrandaCucina;
		bool stop_serranda = tStopSerrandaCucina.update(xChiudiCucina? DELAY_SEC(30) : DELAY_SEC(10), muovi_serranda);
		if (stop_serranda)  {
			xApriCucina = xChiudiCucina = false;
		}

		HW.PompaCalore.xStopPompaCalore->setValue(!xUsaPompaCalore);
		HW.PompaCalore.xRichiestaCaldo->setValue(risc_acceso || xFanCoil);

		if (xUsaCaldaia)
			xCaldaiaInUso = true;
		else if (!xUsaPompaCalore) {
			bool zona_attiva = false;
			zona_attiva |= ((QTime::currentTime()>QTime(6,0)) && (QTime::currentTime()<QTime(8,30)));
			zona_attiva |= ((QTime::currentTime()>QTime(11,0)) && (QTime::currentTime()<QTime(15,0)));
			zona_attiva |= ((QTime::currentTime()>QTime(18,0)) && (QTime::currentTime()<QTime(21,0)));
			if (zona_attiva && (wTemperaturaACS < 500))
				xCaldaiaInUso = true;
			else if (zona_attiva && (wTemperaturaACS > 550))
				xCaldaiaInUso = false;
		} else
			xCaldaiaInUso = false;

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
