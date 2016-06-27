#include "dlg_control.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

enum TrasfAccumulo {
	TrasfAccumuloFermo = 0,
	TrasfDaAccumulo = 1,
	TrasfVersoAccumulo = 2,
};

ControlDlg::ControlDlg(QWidget *parent)
    : QWidget(parent)
    , mLockCount(0)
{
	ui.setupUi(this);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

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

void ControlDlg::on_pbModoRisc_toggled(bool checked)
{
	resetCloseTimer();

	if (checked) {
		ui.pbModoRisc->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbModoCondiz->setPalette(QApplication::palette());
		ui.groupBoxTipo->setTitle("Tipo di riscaldamento");
		ui.pbGiorno->setEnabled(true);
		ui.pbNotte->setEnabled(true);
	} else {
		ui.pbModoRisc->setPalette(QApplication::palette());
		ui.pbModoCondiz->setPalette(QPalette(QColor(64, 64, 255)));
		ui.groupBoxTipo->setTitle("Tipo di condizionamento");
		ui.pbGiorno->setEnabled(false);
		ui.pbNotte->setEnabled(false);
	}

	lockMutex();
	control().xModoRiscaldamento = checked;
	if (!checked) {
		ui.pbGiorno->setChecked(control().xAttivaZonaGiorno = false);
		ui.pbNotte->setChecked(control().xAttivaZonaNotte = false);
	}
	unlockMutex();
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
	control().xAttivaZonaNotte = checked;
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
	control().xAttivaZonaGiorno = checked;
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
	control().xAttivaZonaSoffitta = checked;
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
	control().xAttivaFanCoil = checked;
	unlockMutex();
}

void ControlDlg::updateStatoRisc()
{
	char buf[256];

	if (ui.pbRiscManuale->isChecked()) {
		ui.pbRiscManuale->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscManuale->setText("Manuale");
		ui.pbRiscCaldaia->setEnabled(true);
		ui.pbRiscPompaCalore->setEnabled(true);
		ui.pbRiscResistenze->setEnabled(true);
		ui.pbTrasfAccumulo->setEnabled(true);
	} else {
		ui.pbRiscManuale->setPalette(QApplication::palette());
		ui.pbRiscManuale->setText("Automatico");
		ui.pbRiscCaldaia->setEnabled(false);
		ui.pbRiscPompaCalore->setEnabled(false);
		ui.pbRiscResistenze->setEnabled(false);
		ui.pbTrasfAccumulo->setEnabled(false);
	}

	if (ui.pbRiscCaldaia->isChecked() && ui.pbRiscManuale->isChecked()) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscCaldaia->setText("Caldaia\nON");
	} else if (control().xCaldaiaInUso) {
		ui.pbRiscCaldaia->setPalette(QPalette(QColor(255,192,64)));
		ui.pbRiscCaldaia->setText("Caldaia\nauto ON");
	} else {
		ui.pbRiscCaldaia->setPalette(QApplication::palette());
		ui.pbRiscCaldaia->setText("Caldaia\nOFF");
	}

	if (ui.pbRiscPompaCalore->isChecked() && ui.pbRiscManuale->isChecked()) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscPompaCalore->setText("HPSU\nON");
	} else if (control().xPompaCaloreRiscInUso) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(255,192,64)));
		ui.pbRiscPompaCalore->setText("HPSU\nauto ON");
	} else if (control().xPompaCaloreRiscInUso) {
		ui.pbRiscPompaCalore->setPalette(QPalette(QColor(64,192,255)));
		ui.pbRiscPompaCalore->setText("HPSU\nauto ON");
	} else {
		ui.pbRiscPompaCalore->setPalette(QApplication::palette());
		ui.pbRiscPompaCalore->setText("HPSU\nOFF");
	}

	static bool ultimoResistenzeInUso = false;
	if (ui.pbRiscResistenze->isChecked() && ui.pbRiscManuale->isChecked()) {
		if (ultimoResistenzeInUso && !control().xResistenzeInUso) {
			// fine ciclo: reset bottone automatico
			ui.pbRiscResistenze->setChecked(false);
		} else {
			sprintf(buf, "Resistenze\n%i W", control().wPotResistenze);
			ui.pbRiscResistenze->setPalette(QPalette(QColor(255, 64, 64)));
			ui.pbRiscResistenze->setText(buf);
		}
	} else if (control().xResistenzeInUso) {
		sprintf(buf, "Resistenze\nauto %i W", control().wPotResistenze);
		ui.pbRiscResistenze->setPalette(QPalette(QColor(255,192,64)));
		ui.pbRiscResistenze->setText(buf);
	} else {
		ui.pbRiscResistenze->setPalette(QApplication::palette());
		ui.pbRiscResistenze->setText("Resistenze\nOFF");
	}
	ultimoResistenzeInUso = control().xResistenzeInUso;

	static bool ultimoTrasfAccumulo = false;
	if (ui.pbTrasfAccumulo->isChecked() && ui.pbRiscManuale->isChecked()) {
		if (control().xTrasfDaAccumuloInCorso) {
			ui.pbTrasfAccumulo->setPalette(QPalette(QColor(255,192,64)));
			ui.pbTrasfAccumulo->setText("DA\nAccumulo");
			ultimoTrasfAccumulo = true;
		}
		if (control().xTrasfVersoAccumuloInCorso) {
			ui.pbTrasfAccumulo->setPalette(QPalette(QColor(255,192,64)));
			ui.pbTrasfAccumulo->setText("VERSO\nAccumulo");
			ultimoTrasfAccumulo = true;
		}
	} else if (control().xTrasfDaAccumuloInCorso) {
		ui.pbTrasfAccumulo->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbTrasfAccumulo->setText("DA\nAccumulo");
	} else if (control().xTrasfVersoAccumuloInCorso) {
		ui.pbTrasfAccumulo->setPalette(QPalette(QColor(64, 255, 64)));
		ui.pbTrasfAccumulo->setText("VERSO\nAccumulo");
	} else {
		if (ultimoTrasfAccumulo) {
			control().xTrasfDaAccumulo = false;
			control().xTrasfVersoAccumulo = false;
		}
		ui.pbTrasfAccumulo->setPalette(QApplication::palette());
		ui.pbTrasfAccumulo->setText("Accumulo\nOFF");
		ultimoTrasfAccumulo = false;
	}
}

void ControlDlg::on_pbRiscManuale_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (checked) {
		control().xSetManuale = true;
		/* recover from current state */
		ui.pbRiscCaldaia->setChecked(control().xUsaCaldaia = control().xCaldaiaInUso);
		ui.pbRiscPompaCalore->setChecked(control().xUsaPompaCalore = (control().xPompaCaloreRiscInUso || control().xPompaCaloreCondInUso));
		ui.pbRiscResistenze->setChecked(control().xUsaResistenze = control().xResistenzeInUso);
		control().xTrasfDaAccumulo = control().xTrasfDaAccumuloInCorso;
		control().xTrasfVersoAccumulo = control().xTrasfVersoAccumuloInCorso;
		ui.pbTrasfAccumulo->setChecked(control().xTrasfDaAccumulo || control().xTrasfVersoAccumulo);
	} else {
		control().xSetManuale = false;
		ui.pbRiscCaldaia->setChecked(false);
		ui.pbRiscPompaCalore->setChecked(false);
		ui.pbRiscResistenze->setChecked(false);
		ui.pbTrasfAccumulo->setChecked(false);
	}
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

void ControlDlg::on_pbTrasfAccumulo_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (!control().xTrasfDaAccumulo && !control().xTrasfVersoAccumulo) {
		// fermo->da
		control().xTrasfDaAccumulo = true;
		control().xTrasfVersoAccumulo = false;
		ui.pbTrasfAccumulo->setChecked(true);
	} else if (control().xTrasfDaAccumulo && !control().xTrasfVersoAccumulo) {
		// da->verso
		control().xTrasfDaAccumulo = false;
		control().xTrasfVersoAccumulo = true;
		ui.pbTrasfAccumulo->setChecked(true);
	} else if (!control().xTrasfDaAccumulo && control().xTrasfVersoAccumulo) {
		// verso->off
		control().xTrasfDaAccumulo = false;
		control().xTrasfVersoAccumulo = false;
		ui.pbTrasfAccumulo->setChecked(false);
	}
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

	updateStatoRisc();

	unlockMutex();
}
