#include "dlg_main.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

MainDlg::MainDlg(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	connect(&screenUpdate, SIGNAL( timeout(void) ), this, SLOT ( updateScreen(void) ));
	screenUpdate.setSingleShot(false);
	screenUpdate.start(100);
}

void MainDlg::on_pbNotte_toggled(bool checked)
{
	if (checked) {
		ui.pbNotte->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbNotte->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xRiscaldaNotte = checked;
}

void MainDlg::on_pbGiorno_toggled(bool checked)
{
	if (checked) {
		ui.pbGiorno->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbGiorno->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xRiscaldaGiorno = checked;
}

void MainDlg::on_pbSoffitta_toggled(bool checked)
{
	if (checked) {
		ui.pbSoffitta->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbSoffitta->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xRiscaldaSoffitta = checked;
}

void MainDlg::on_pbFanCoil_toggled(bool checked)
{
	if (checked) {
		ui.pbFanCoil->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbFanCoil->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xFanCoil = checked;
}

void MainDlg::on_pbRiscCaldaia_toggled(bool checked)
{
	if (checked) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscCaldaia->setText("Caldaia ON");
		ui.pbRiscPompaCalore->setChecked(false);
	} else {
		ui.pbRiscCaldaia->setPalette(QApplication::palette());
		ui.pbRiscCaldaia->setText("Caldaia OFF");
	}

	QMutexLocker lock(&control().mFields);
	control().xUsaCaldaia = checked;
}

void MainDlg::on_pbRiscPompaCalore_toggled(bool checked)
{
	if (checked) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(64, 255, 64)));
		ui.pbRiscPompaCalore->setText("Pompa di calore ON");
		ui.pbRiscCaldaia->setChecked(false);
	} else {
		ui.pbRiscPompaCalore->setPalette(QApplication::palette());
		ui.pbRiscPompaCalore->setText("Pompa di calore OFF");
	}

	QMutexLocker lock(&control().mFields);
	control().xUsaPompaCalore = checked;
}

void MainDlg::on_pbVelMinus_clicked()
{
	QMutexLocker lock(&control().mFields);
	if (control().wVelFanCoil > 5)
		control().wVelFanCoil -= 5;
	else
		control().wVelFanCoil = 0;

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void MainDlg::on_pbVelPlus_clicked()
{
	QMutexLocker lock(&control().mFields);
	if (control().wVelFanCoil < 95)
		control().wVelFanCoil += 5;
	else
		control().wVelFanCoil = 100;

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void MainDlg::on_pbTrasfAccumulo_toggled(bool checked)
{
	if (checked) {
		ui.pbTrasfAccumulo->setPalette(QPalette(QColor(64, 255, 64)));
	} else {
		ui.pbTrasfAccumulo->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xTrasfAccumulo = checked;
}

void MainDlg::on_pbApriCucina_clicked()
{
	QMutexLocker lock(&control().mFields);
	control().xApriCucina = true;
}

void MainDlg::on_pbChiudiCucina_clicked()
{
	QMutexLocker lock(&control().mFields);
	control().xChiudiCucina = true;
}

void MainDlg::updateScreen()
{
	char buf[256];
	sprintf(buf, "%.1f", HW.PompaCalore.wTemperaturaACS->getValue()/10.0);
	ui.tlTempAcquaTop->setText(buf);
	sprintf(buf, "%.1f", HW.PompaCalore.wTemperaturaBoiler->getValue()/10.0);
	ui.tlTempAcquaBot->setText(buf);
	sprintf(buf, "%.1f", HW.Accumuli.wTemperatura->getValue()/10.0);
	ui.tlTempAccumulo->setText(buf);

	QMutexLocker lock(&control().mFields);
	if (control().xApriCucina) {
		ui.pbApriCucina->setPalette(QPalette(QColor(64, 255, 64)));
	} else {
		ui.pbApriCucina->setPalette(QApplication::palette());
	}
	if (control().xChiudiCucina) {
		ui.pbChiudiCucina->setPalette(QPalette(QColor(64, 255, 64)));
	} else {
		ui.pbChiudiCucina->setPalette(QApplication::palette());
	}
}
