#include "hardware.h"
#include "modbusdevice.h"

#include "dlg_main.h"

#include <QtGui>

int main(int argc, char *argv[])
{
	ModbusDevice::openSerial("/dev/ttymxc2", 115200, 'N', 8, 1);
	InitHardware();

	ReadHardwareInputs();

	WriteHardwareOutputs();

	QApplication app(argc, argv);

	MainDlg *mainDlg = new MainDlg;
	mainDlg->show();

	return app.exec();
}
