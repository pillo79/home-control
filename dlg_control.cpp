#include "dlg_control.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

ControlDlg::ControlDlg(QWidget *parent)
    : QWidget(parent)
    , mLockCount(0)
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

void ControlDlg::lockMutex()
{
	if (!mLockCount)
		control().mFields.lock();
	mLockCount++;
}

void ControlDlg::unlockMutex()
{
	mLockCount--;
	if (!mLockCount)
		control().mFields.unlock();
}

void ControlDlg::on_pbNotte_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbNotte->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbNotte->setPalette(QApplication::palette());
	}

	lockMutex();
	control().xRiscaldaNotte = checked;
	unlockMutex();
}

void ControlDlg::on_pbGiorno_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbGiorno->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbGiorno->setPalette(QApplication::palette());
	}

	lockMutex();
	control().xRiscaldaGiorno = checked;
	unlockMutex();
}

void ControlDlg::on_pbSoffitta_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbSoffitta->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbSoffitta->setPalette(QApplication::palette());
	}

	lockMutex();
	control().xRiscaldaSoffitta = checked;
	unlockMutex();
}

void ControlDlg::on_pbFanCoil_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbFanCoil->setPalette(QPalette(QColor(255, 64, 64)));
	} else {
		ui.pbFanCoil->setPalette(QApplication::palette());
	}

	lockMutex();
	control().xFanCoil = checked;
	unlockMutex();
}

void ControlDlg::updateStatoRisc()
{
	if (ui.pbRiscManuale->isChecked()) {
		ui.pbRiscManuale->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscManuale->setText("Manuale");
		ui.pbRiscCaldaia->setEnabled(true);
		ui.pbRiscPompaCalore->setEnabled(true);
		ui.pbRiscResistenze->setEnabled(true);
	} else {
		ui.pbRiscManuale->setPalette(QApplication::palette());
		ui.pbRiscManuale->setText("Automatico");
		ui.pbRiscCaldaia->setEnabled(false);
		ui.pbRiscPompaCalore->setEnabled(false);
		ui.pbRiscResistenze->setEnabled(false);
	}

	if (ui.pbRiscCaldaia->isChecked() && ui.pbRiscManuale->isChecked()) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscCaldaia->setText("Caldaia\nON");
	} else if (control().xCaldaiaInUso) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscCaldaia->setText("Caldaia\nauto ON");
	} else {
		ui.pbRiscCaldaia->setPalette(QApplication::palette());
		ui.pbRiscCaldaia->setText("Caldaia\nOFF");
	}

	if (ui.pbRiscPompaCalore->isChecked() && ui.pbRiscManuale->isChecked()) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(64, 255, 64)));
		ui.pbRiscPompaCalore->setText("Pompa di calore\nON");
	} else if (control().xPompaCaloreInUso) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscPompaCalore->setText("Pompa di calore\nauto ON");
	} else {
		ui.pbRiscPompaCalore->setPalette(QApplication::palette());
		ui.pbRiscPompaCalore->setText("Pompa di calore\nOFF");
	}

	static bool ultimoResistenzeInUso = false;
	if (ui.pbRiscResistenze->isChecked() && ui.pbRiscManuale->isChecked()) {
		if (ultimoResistenzeInUso && !control().xResistenzeInUso) {
			// fine ciclo: reset bottone automatico
			ui.pbRiscResistenze->setChecked(false);
		} else {
			ui.pbRiscResistenze->setPalette(QPalette(QColor(255, 64, 64)));
			ui.pbRiscResistenze->setText("Resistenze\nON");
		}
	} else if (control().xResistenzeInUso) {
		ui.pbRiscResistenze->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscResistenze->setText("Resistenze\nauto ON");
	} else {
		ui.pbRiscResistenze->setPalette(QApplication::palette());
		ui.pbRiscResistenze->setText("Resistenze\nOFF");
	}
	ultimoResistenzeInUso = control().xResistenzeInUso;
}

void ControlDlg::on_pbRiscManuale_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xSetManuale = checked;
	updateStatoRisc();
	unlockMutex();
}

void ControlDlg::on_pbRiscCaldaia_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xUsaCaldaia = checked;
	updateStatoRisc();
	unlockMutex();
}

void ControlDlg::on_pbRiscPompaCalore_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xUsaPompaCalore = checked;
	updateStatoRisc();
	unlockMutex();
}

void ControlDlg::on_pbRiscResistenze_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xUsaResistenze = checked;
	updateStatoRisc();
	unlockMutex();
}

void ControlDlg::on_pbVelMinus_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (control().wVelFanCoil > 5)
		control().wVelFanCoil -= 5;
	else
		control().wVelFanCoil = 0;
	unlockMutex();

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void ControlDlg::on_pbVelPlus_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (control().wVelFanCoil < 95)
		control().wVelFanCoil += 5;
	else
		control().wVelFanCoil = 100;
	unlockMutex();

	char buf[256];
	sprintf(buf, "%i%%", control().wVelFanCoil);
	ui.tlVelFanCoil->setText(buf);
}

void ControlDlg::on_pbTrasfAccumulo_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xTrasfAccumulo = checked;
	unlockMutex();
}

void ControlDlg::on_pbApriCucina_clicked()
{
	resetCloseTimer();

	lockMutex();
	control().xApriCucina = true;
	unlockMutex();
}

void ControlDlg::on_pbChiudiCucina_clicked()
{
	resetCloseTimer();

	lockMutex();
	control().xChiudiCucina = true;
	unlockMutex();
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

	lockMutex();
	sprintf(buf, "%.1f", control().wTemperaturaACS/10.0);
	ui.tlTempAcquaTop->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaBoiler/10.0);
	ui.tlTempAcquaBot->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaAccumulo/10.0);
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

	static bool ultimoTrasfAccumulo = false;
	if (control().xTrasfAccumuloInCorso) {
		ui.pbTrasfAccumulo->setPalette(QPalette(QColor(64, 255, 64)));
		ultimoTrasfAccumulo = true;
	} else {
		if (ultimoTrasfAccumulo)
			ui.pbTrasfAccumulo->setChecked(false);
		ui.pbTrasfAccumulo->setPalette(QApplication::palette());
		ultimoTrasfAccumulo = false;
	}

	updateStatoRisc();

	unlockMutex();
}
