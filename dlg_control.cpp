#include "dlg_control.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

ControlDlg::ControlDlg(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	ui.pbRiscPompaCalore->setChecked(true);

	connect(&m_closeTimer, SIGNAL( timeout(void) ), this, SLOT ( hide(void) ));

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void ControlDlg::on_pbNotte_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbNotte->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbNotte->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xRiscaldaNotte = checked;
}

void ControlDlg::on_pbGiorno_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbGiorno->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbGiorno->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xRiscaldaGiorno = checked;
}

void ControlDlg::on_pbSoffitta_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbSoffitta->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbSoffitta->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xRiscaldaSoffitta = checked;
}

void ControlDlg::on_pbFanCoil_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbFanCoil->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbFanCoil->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xFanCoil = checked;
}

void ControlDlg::updateStatoRisc()
{
	if (ui.pbRiscCaldaia->isChecked()) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscCaldaia->setText("Caldaia ON");
	} else if (control().xCaldaiaInUso) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscCaldaia->setText("Caldaia auto ON");
	} else {
		ui.pbRiscCaldaia->setPalette(QApplication::palette());
		ui.pbRiscCaldaia->setText("Caldaia OFF");
	}

	if (ui.pbRiscPompaCalore->isChecked()) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(64, 255, 64)));
		ui.pbRiscPompaCalore->setText("Pompa di calore ON");
	} else if (control().xPompaCaloreInUso) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscPompaCalore->setText("Pompa di calore auto ON");
	} else {
		ui.pbRiscPompaCalore->setPalette(QApplication::palette());
		ui.pbRiscPompaCalore->setText("Pompa di calore OFF");
	}
}

void ControlDlg::on_pbRiscCaldaia_toggled(bool checked)
{
	resetCloseTimer();

	QMutexLocker lock(&control().mFields);
	control().xUsaCaldaia = checked;
	updateStatoRisc();
}

void ControlDlg::on_pbRiscPompaCalore_toggled(bool checked)
{
	resetCloseTimer();

	QMutexLocker lock(&control().mFields);
	control().xUsaPompaCalore = checked;
	updateStatoRisc();
}

void ControlDlg::on_pbVelMinus_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&control().mFields);
	if (control().wVelFanCoil > 5)
		control().wVelFanCoil -= 5;
	else
		control().wVelFanCoil = 0;

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void ControlDlg::on_pbVelPlus_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&control().mFields);
	if (control().wVelFanCoil < 95)
		control().wVelFanCoil += 5;
	else
		control().wVelFanCoil = 100;

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void ControlDlg::on_pbTrasfAccumulo_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbTrasfAccumulo->setPalette(QPalette(QColor(64, 255, 64)));
	} else {
		ui.pbTrasfAccumulo->setPalette(QApplication::palette());
	}

	QMutexLocker lock(&control().mFields);
	control().xTrasfAccumulo = checked;
}

void ControlDlg::on_pbApriCucina_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&control().mFields);
	control().xApriCucina = true;
}

void ControlDlg::on_pbChiudiCucina_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&control().mFields);
	control().xChiudiCucina = true;
}

void ControlDlg::resetCloseTimer()
{
	m_closeTimer.setSingleShot(true);
	m_closeTimer.start(60000);
}

void ControlDlg::on_pbOK_clicked()
{
	hide();
}

void ControlDlg::updateScreen()
{
	char buf[256];

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

	updateStatoRisc();
}
