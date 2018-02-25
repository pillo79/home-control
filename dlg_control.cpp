#include "dlg_control.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

ControlDlg::ControlDlg(QWidget *parent)
    : QWidget(parent)
    , mSettings("/media/mmcblk0p2/settings.ini", QSettings::IniFormat)
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

	mDisabledColor = QColor(96, 96, 96);
	mForcedNormColor = QColor(220, 220, 64);
	mAutoNormColor = QColor(255, 255, 128);
	mForcedRiscColor = QColor(255, 64, 64);
	mAutoRiscColor = QColor(255, 160, 96);
	mForcedCondColor = QColor(64, 64, 255);
	mAutoCondColor = QColor(96, 160, 255);

	loadSettings();
}

void ControlDlg::loadSettings()
{
	ui.pbModoRisc->setChecked(mSettings.value("pbModoRisc").toBool());

	ui.pbNotte->setChecked(mSettings.value("pbNotte").toBool());
	ui.pbGiorno->setChecked(mSettings.value("pbGiorno").toBool());
	ui.pbSoffitta->setChecked(mSettings.value("pbSoffitta").toBool());
	ui.pbFanCoil->setChecked(mSettings.value("pbFanCoil").toBool());

	ui.pbRiscManuale->setChecked(mSettings.value("pbRiscManuale").toBool());
	ui.pbRiscCaldaia->setChecked(mSettings.value("pbRiscCaldaia").toBool());
	ui.pbRiscPompaCalore->setChecked(mSettings.value("pbRiscPompaCalore").toBool());
	ui.pbRiscResistenze->setChecked(mSettings.value("pbRiscResistenze").toBool());

	ui.pbForzaChiudi->setChecked(mSettings.value("pbForzaChiudi").toBool());

	lockMutex();

	control().wVelFanCoil = mSettings.value("wVelFanCoil").toInt();
	control().wApriCucinaPerc = mSettings.value("wApriCucinaPerc").toInt();

	updateBtnStatus();

	unlockMutex();
}

void ControlDlg::saveSettings()
{
	mSettings.setValue("pbModoRisc", ui.pbModoRisc->isChecked());

	mSettings.setValue("pbNotte", ui.pbNotte->isChecked());
	mSettings.setValue("pbGiorno", ui.pbGiorno->isChecked());
	mSettings.setValue("pbSoffitta", ui.pbSoffitta->isChecked());
	mSettings.setValue("pbFanCoil", ui.pbFanCoil->isChecked());

	mSettings.setValue("pbRiscManuale", ui.pbRiscManuale->isChecked());
	mSettings.setValue("pbRiscCaldaia", ui.pbRiscCaldaia->isChecked());
	mSettings.setValue("pbRiscPompaCalore", ui.pbRiscPompaCalore->isChecked());
	mSettings.setValue("pbRiscResistenze", ui.pbRiscResistenze->isChecked());

	mSettings.setValue("pbForzaChiudi", ui.pbForzaChiudi->isChecked());

	mSettings.setValue("wVelFanCoil", control().wVelFanCoil);
	mSettings.setValue("wApriCucinaPerc", control().wApriCucinaPerc);
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
		ui.groupBoxTipo->setTitle("Tipo di riscaldamento");
		ui.pbGiorno->setEnabled(true);
		ui.pbNotte->setEnabled(true);
	} else {
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
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbNotte_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xAttivaZonaNotte = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbGiorno_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xAttivaZonaGiorno = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbSoffitta_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xAttivaZonaSoffitta = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbFanCoil_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xAttivaFanCoil = checked;
	updateBtnStatus();
	unlockMutex();

	if (!checked && ui.pbForzaChiudi->isChecked())
		ui.pbForzaChiudi->setChecked(false);
}

void ControlDlg::setBtnStatus(QPushButton *pb, bool state, ButtonColor mode, QString forced, QString automatic, QString off, bool disable, QString disabled)
{
	if (disable) {
		if (!disabled.isEmpty()) pb->setText(disabled);
		pb->setPalette(QPalette(mDisabledColor));
	} else if (pb->isChecked()) {
		if (!forced.isEmpty()) pb->setText(forced);
		switch(mode) {
			case bcNorm: pb->setPalette(QPalette(mForcedNormColor)); break;
			case bcCond: pb->setPalette(QPalette(mForcedCondColor)); break;
			case bcRisc: pb->setPalette(QPalette(mForcedRiscColor)); break;
			case bcAuto: pb->setPalette(QPalette(mForcedColor)); break;
		}
	} else if (state) {
		if (!automatic.isEmpty()) pb->setText(automatic);
		switch(mode) {
			case bcNorm: pb->setPalette(QPalette(mAutoNormColor)); break;
			case bcCond: pb->setPalette(QPalette(mAutoCondColor)); break;
			case bcRisc: pb->setPalette(QPalette(mAutoRiscColor)); break;
			case bcAuto: pb->setPalette(QPalette(mAutoColor)); break;
		}
	} else {
		if (!off.isEmpty()) pb->setText(off);
		pb->setPalette(QApplication::palette());
	}
}

void ControlDlg::setBtnStatus3Way(QPushButton *pb, bool stateRisc, bool stateCond, ButtonColor mode, QString forceRisc, QString forceCond, QString autoRisc, QString autoCond, QString off, bool disable, QString disabled)
{
	if (disable) {
		if (!disabled.isEmpty()) pb->setText(disabled);
		pb->setPalette(QPalette(mDisabledColor));
	} else if (pb->isChecked()) {
		if (stateRisc) {
			if (!forceRisc.isEmpty()) pb->setText(forceRisc);
			switch(mode) {
				case bcNorm: pb->setPalette(QPalette(mForcedNormColor)); break;
				case bcCond: pb->setPalette(QPalette(mForcedCondColor)); break;
				case bcRisc: pb->setPalette(QPalette(mForcedRiscColor)); break;
				case bcAuto: pb->setPalette(QPalette(mForcedRiscColor)); break;
			}
		} else if (stateCond) {
			if (!forceCond.isEmpty()) pb->setText(forceCond);
			switch(mode) {
				case bcNorm: pb->setPalette(QPalette(mForcedNormColor)); break;
				case bcCond: pb->setPalette(QPalette(mForcedCondColor)); break;
				case bcRisc: pb->setPalette(QPalette(mForcedRiscColor)); break;
				case bcAuto: pb->setPalette(QPalette(mForcedCondColor)); break;
			}
		} else {
			if (!off.isEmpty()) pb->setText(off);
			pb->setPalette(QApplication::palette());
		}
	} else if (stateRisc) {
		if (!autoRisc.isEmpty()) pb->setText(autoRisc);
		switch(mode) {
			case bcNorm: pb->setPalette(QPalette(mAutoNormColor)); break;
			case bcCond: pb->setPalette(QPalette(mAutoCondColor)); break;
			case bcRisc: pb->setPalette(QPalette(mAutoRiscColor)); break;
			case bcAuto: pb->setPalette(QPalette(mAutoRiscColor)); break;
		}
	} else if (stateCond) {
		if (!autoCond.isEmpty()) pb->setText(autoCond);
		switch(mode) {
			case bcNorm: pb->setPalette(QPalette(mAutoNormColor)); break;
			case bcCond: pb->setPalette(QPalette(mAutoCondColor)); break;
			case bcRisc: pb->setPalette(QPalette(mAutoRiscColor)); break;
			case bcAuto: pb->setPalette(QPalette(mAutoCondColor)); break;
		}
	} else {
		if (!off.isEmpty()) pb->setText(off);
		pb->setPalette(QApplication::palette());
	}
}

void ControlDlg::updateBtnStatus()
{
	char buf1[256], buf2[256];

	bool manuale = ui.pbRiscManuale->isChecked();

	if (ui.pbModoRisc->isChecked()) {
		mForcedColor = mForcedRiscColor;
		mAutoColor = mAutoRiscColor;
	} else {
		mForcedColor = mForcedCondColor;
		mAutoColor = mAutoCondColor;
	}

	setBtnStatus(ui.pbModoRisc, false);
	setBtnStatus(ui.pbModoCondiz, false);

	setBtnStatus(ui.pbNotte, false);
	setBtnStatus(ui.pbGiorno, false);
	setBtnStatus(ui.pbSoffitta, false);
	setBtnStatus(ui.pbFanCoil, false);

	setBtnStatus(ui.pbRiscManuale, false, bcNorm, "Manuale", "", "Automatico");

	setBtnStatus(ui.pbRiscCaldaia, control().xCaldaiaInUso, bcRisc, "Caldaia\nON", "Caldaia\nauto ON", "Caldaia\nOFF", !manuale && control().xDisabilitaCaldaia, "BLOCCO\nCaldaia");
	setBtnStatus3Way(ui.pbRiscPompaCalore, control().xPompaCaloreRiscInUso, control().xPompaCaloreCondInUso, bcAuto, "HPSU\nON", "HPSU\nON", "HPSU\nauto ON", "HPSU\nauto ON", "HPSU\nOFF", !manuale && control().xDisabilitaPompaCalore, "BLOCCO\nHPSU");

	sprintf(buf1, "Resistenze\n%.0f W", control().wPotResistenze.value());
	sprintf(buf2, "Resistenze\nauto %.0f W", control().wPotResistenze.value());
	setBtnStatus(ui.pbRiscResistenze, control().xResistenzeInUso, bcRisc, buf1, buf2, "Resistenze\nOFF", !manuale && control().xDisabilitaResistenze, "BLOCCO\nResistenze");

	setBtnStatus(ui.pbForzaChiudi, false, bcNorm, "Mantieni chiusa", "", "Apri quando serve");

	setBtnStatus3Way(ui.pbTrasfAccumulo, control().xTrasfDaAccumuloInCorso, control().xTrasfVersoAccumuloInCorso, bcNorm, "DA\nAccumulo", "VERSO\nAccumulo", "DA\nAccumulo", "VERSO\nAccumulo", "Accumulo\nOFF", !manuale && control().xDisabilitaAccumulo, "BLOCCO\nAccumulo");

	static bool ultimoResistenzeInUso = false;
	if (ui.pbRiscResistenze->isChecked() && ultimoResistenzeInUso && !control().xResistenzeInUso) {
		// fine ciclo: reset bottone automatico
		ui.pbRiscResistenze->setChecked(false);
	}
	ultimoResistenzeInUso = control().xResistenzeInUso;

	static bool ultimoTrasfAccumulo = false;
	if (control().xTrasfDaAccumuloInCorso || control().xTrasfVersoAccumuloInCorso) {
		if (ui.pbTrasfAccumulo->isChecked() && ui.pbRiscManuale->isChecked())
			ultimoTrasfAccumulo = true;
	} else {
		if (ultimoTrasfAccumulo) {
			control().xTrasfDaAccumulo = false;
			control().xTrasfVersoAccumulo = false;
		}
		ultimoTrasfAccumulo = false;
	}

	saveSettings();
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
		ui.pbRiscCaldaia->setChecked(control().xDisabilitaCaldaia);
		ui.pbRiscPompaCalore->setChecked(control().xDisabilitaPompaCalore);
		ui.pbRiscResistenze->setChecked(control().xDisabilitaResistenze);
		ui.pbTrasfAccumulo->setChecked(control().xDisabilitaAccumulo);
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbRiscCaldaia_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		control().xUsaCaldaia = checked;
	} else {
		control().xDisabilitaCaldaia = checked;
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbRiscPompaCalore_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		control().xUsaPompaCalore = checked;
	} else {
		control().xDisabilitaPompaCalore = checked;
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbRiscResistenze_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		control().xUsaResistenze = checked;
	} else {
		control().xDisabilitaResistenze = checked;
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbTrasfAccumulo_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
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
	} else {
		control().xDisabilitaAccumulo = ui.pbTrasfAccumulo->isChecked();
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

void ControlDlg::on_pbApriPlus_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (control().wApriCucinaPerc < 90)
		control().wApriCucinaPerc += 10;
	else
		control().wApriCucinaPerc = 100;
	unlockMutex();

	char buf[256];
	sprintf(buf, "%i%%", control().wApriCucinaPerc);
	ui.tlApriCucinaPerc->setText(buf);
}

void ControlDlg::on_pbApriMinus_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (control().wApriCucinaPerc > 10)
		control().wApriCucinaPerc -= 10;
	else
		control().wApriCucinaPerc = 0;
	unlockMutex();

	char buf[256];
	sprintf(buf, "%i%%", control().wApriCucinaPerc);
	ui.tlApriCucinaPerc->setText(buf);
}

void ControlDlg::on_pbForzaChiudi_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	control().xForzaChiudi = checked;
	updateBtnStatus();
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
	sprintf(buf, "%.1f", control().wTemperaturaACS.value());
	ui.tlTempAcquaTop->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaBoiler.value());
	ui.tlTempAcquaBot->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaAccumulo.value());
	ui.tlTempAccumulo->setText(buf);

	static const QTime MIDNIGHT = QTime(0, 0, 10);
	if (QTime::currentTime() < MIDNIGHT) {
		// operations to be performed every midnight
		if (ui.pbRiscManuale->isChecked())
			ui.pbRiscManuale->setChecked(false);
	}

	updateBtnStatus();

	unlockMutex();
}
