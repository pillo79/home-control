#include "hardware.h"
#include "modbusdevice.h"
#include "hardware.h"

#include "control.h"
#include "dlg_main.h"

#include <QtGui>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	control();

	MainDlg *mainDlg = new MainDlg;
	mainDlg->show();

	return app.exec();

	return 0;
}
