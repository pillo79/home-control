#include "hardware.h"
#include "modbusdevice.h"
#include "hardware.h"

#include "state.h"
#include "control.h"
#include "dlg_control.h"
#include "dlg_status.h"

#include <QtGui>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	s();

	control();

	ControlDlg *controlDlg = new ControlDlg;
	StatusDlg *statusDlg = new StatusDlg(NULL, controlDlg);

	QTimer screenUpdate;
	QObject::connect(&screenUpdate, SIGNAL( timeout(void) ), controlDlg, SLOT ( updateScreen(void) ));
	QObject::connect(&screenUpdate, SIGNAL( timeout(void) ), statusDlg, SLOT ( updateScreen(void) ));

	screenUpdate.setSingleShot(false);
	screenUpdate.start(100);

	statusDlg->show();
	return app.exec();

	return 0;
}
