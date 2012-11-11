#include "hardware.h"
#include "modbusdevice.h"
#include "hardware.h"

#include "dlg_main.h"

#include <QtGui>

int main(int argc, char *argv[])
{
	ModbusDevice::openSerial("/dev/ttymxc1", 38400, 'N', 8, 1);
	InitHardware();

	ReadHardwareInputs();

	HW.FanCoilCorridoio.xStartVentilatore->setValue(1);
	HW.FanCoilCorridoio.wLivelloVentilatore->setValue(500);
	WriteHardwareOutputs();

/*	QApplication app(argc, argv);

	MainDlg *mainDlg = new MainDlg;
	mainDlg->show();

	return app.exec();
*/
	return 0;
}
