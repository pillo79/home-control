#include "control.h"

#include "modbusdevice.h"
#include "hardware.h"
#include "plc.h"

ControlThread::ControlThread()
{
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

		HW.PompaCalore.xStopPompaCalore->setValue(!xUsaPompaCalore);
		HW.PompaCalore.xRichiestaCaldo->setValue(risc_acceso || xFanCoil);

		HW.Caldaia.xAlimenta->setValue(xUsaCaldaia);
		static DelayRiseTimer tStartCaldaia;
		HW.Caldaia.xStartCaldaia->setValue(tStartCaldaia.update(DELAY_SEC(10), xUsaCaldaia));
		static DelayFallTimer tStartPompa;
		HW.Caldaia.xStartPompa->setValue(tStartPompa.update(DELAY_SEC(60), HW.Caldaia.xStartCaldaia->getValue()));

		mFields.unlock();

		WriteHardwareOutputs();
	}
}

ControlThread &control()
{
	static ControlThread theControl;
	return theControl;
}
