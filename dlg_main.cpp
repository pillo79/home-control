#include "dlg_main.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

MainDlg::MainDlg(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

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

void MainDlg::updateRiscCaldaia()
{
	if (ui.pbRiscCaldaia->isChecked()) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscCaldaia->setText("Caldaia ON");
		ui.pbRiscPompaCalore->setChecked(false);
	} else if (control().xCaldaiaInUso) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscCaldaia->setText("Caldaia auto ON");
	} else {
		ui.pbRiscCaldaia->setPalette(QApplication::palette());
		ui.pbRiscCaldaia->setText("Caldaia OFF");
	}
}

void MainDlg::on_pbRiscCaldaia_toggled(bool checked)
{
	QMutexLocker lock(&control().mFields);
	control().xUsaCaldaia = checked;
	updateRiscCaldaia();
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

	ui.tlData->setText(QDate::currentDate().toString("dd/MM/yyyy"));
	ui.tlOra->setText(QTime::currentTime().toString("h:mm"));

	QMutexLocker lock(&control().mFields);
	sprintf(buf, "%.1f", control().wTemperaturaACS/10.0);
	ui.tlTempAcquaTop->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaBoiler/10.0);
	ui.tlTempAcquaBot->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaAccumuli/10.0);
	ui.tlTempAccumulo->setText(buf);

	if (control().xCaldaiaInUso) {
		ui.pbApriCucina->setPalette(QPalette(QColor(64, 255, 64)));
	} else {
		ui.pbApriCucina->setPalette(QApplication::palette());
	}
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

	updateRiscCaldaia();
}