#include "dlg_control.h"

#include "control.h"
#include "hardware.h"

#include "state.h"

#include "ctrlvalue.h"
#include "ctrlobserver.h"

#include <QtGui>
#include <QApplication>

ControlDlg::ControlDlg(QWidget *parent)
    : QWidget(parent)
    , mSettings("/media/mmcblk0p2/settings.ini", QSettings::IniFormat)
    , mLockCount(0)
{
	ui.setupUi(this);
	/*
	for (int t=0; t<3; t++) {
		for (int hr=0; hr<8; ++hr) {
			ui.progGridLayout->addWidget(new QLabel(QString::number(hr+t*8)), hr, t*3);
			QPushButton *pb = new QPushButton();
			pb->setCheckable(true);
			pb->setAutoExclusive(false);
			ui.progGridLayout->addWidget(pb, hr, t*3+1, 1, 2);
			mProgEn[hr+t*8] = pb;
		}
	}
	*/

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	m_closeTimer.setSingleShot(true);
	connect(&m_closeTimer, SIGNAL( timeout(void) ), this, SLOT ( hide(void) ));

	char buf[256];
	sprintf(buf, "%i%%", s().wVelFanCoil.value());
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
	if (s().xModoRiscaldamento) {
		ui.pbModoRisc->setChecked(true);
	} else {
		ui.pbModoCondiz->setChecked(true);
	}

	ui.pbNotte->setChecked(s().xAttivaZonaNotte);
	ui.pbGiorno->setChecked(s().xAttivaZonaGiorno);
	ui.pbSoffitta->setChecked(s().xAttivaZonaSoffitta);
	ui.pbFanCoil->setChecked(s().xAttivaFanCoil);

	ui.pbRiscManuale->setChecked(s().xSetManuale);
	ui.pbRiscGas->setChecked(s().xUsaGas);
	ui.pbRiscPompaCalore->setChecked(s().xUsaPompaCalore);
	ui.pbRiscResistenze->setChecked(s().xUsaResistenze);

	ui.pbForzaChiudi->setChecked(s().xForzaChiudi);

	lockMutex();
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::lockMutex()
{
	if (!mLockCount)
		s().mFields.lock();
	mLockCount++;
}

void ControlDlg::unlockMutex()
{
	mLockCount--;
	if (!mLockCount)
		s().mFields.unlock();
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
	s().xModoRiscaldamento(O_UI_CTRL) = checked;
	if (!checked) {
		ui.pbGiorno->setChecked(s().xAttivaZonaGiorno(O_UI_CTRL) = false);
		ui.pbNotte->setChecked(s().xAttivaZonaNotte(O_UI_CTRL) = false);
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbNotte_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	s().xAttivaZonaNotte(O_UI_CTRL) = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbGiorno_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	s().xAttivaZonaGiorno(O_UI_CTRL) = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbSoffitta_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	s().xAttivaZonaSoffitta(O_UI_CTRL) = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbFanCoil_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	s().xAttivaFanCoil(O_UI_CTRL) = checked;
	updateBtnStatus();
	unlockMutex();

	if (!checked && ui.pbForzaChiudi->isChecked())
		ui.pbForzaChiudi->setChecked(false);
}

void ControlDlg::on_pbProg_toggled(bool checked)
{
	lockMutex();
	s().xAttivaProg(O_UI_CTRL) = checked;
	updateBtnStatus();
	unlockMutex();
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
	ButtonColor mode;

	if (ui.pbModoRisc->isChecked()) {
		mForcedColor = mForcedRiscColor;
		mAutoColor = mAutoRiscColor;
		mode = bcRisc;
	} else {
		mForcedColor = mForcedCondColor;
		mAutoColor = mAutoCondColor;
		mode = bcCond;
	}

	setBtnStatus(ui.pbModoRisc, false);
	setBtnStatus(ui.pbModoCondiz, false);

	setBtnStatus(ui.pbNotte, false, s().xImpiantoAttivo? mode : bcNorm);
	setBtnStatus(ui.pbGiorno, false, s().xImpiantoAttivo? mode : bcNorm);
	setBtnStatus(ui.pbSoffitta, false, s().xImpiantoAttivo? mode : bcNorm);
	setBtnStatus(ui.pbFanCoil, false, s().xImpiantoAttivo? mode : bcNorm);
	setBtnStatus(ui.pbProg, false, bcNorm, "Attivo", "", "Non attivo");

	setBtnStatus(ui.pbRiscManuale, false, bcNorm, "Manuale", "", "Automatico");

	setBtnStatus(ui.pbRiscGas, s().xGasInUso, bcRisc, "Gas\nON", "Gas\nauto ON", "Gas\nOFF", !manuale && s().xDisabilitaGas, "BLOCCO\nGas");
	setBtnStatus3Way(ui.pbRiscPompaCalore, s().xPompaCaloreRiscInUso, s().xPompaCaloreCondInUso, bcAuto, "HPSU\nON", "HPSU\nON", "HPSU\nauto ON", "HPSU\nauto ON", "HPSU\nOFF", !manuale && s().xDisabilitaPompaCalore, "BLOCCO\nHPSU");

	sprintf(buf1, "Resistenze\n%.0f W", s().wPotResistenze.value());
	sprintf(buf2, "Resistenze\nauto %.0f W", s().wPotResistenze.value());
	setBtnStatus(ui.pbRiscResistenze, s().xResistenzeInUso, bcRisc, buf1, buf2, "Resistenze\nOFF", !manuale && s().xDisabilitaResistenze, "BLOCCO\nResistenze");

	setBtnStatus(ui.pbForzaChiudi, false, bcNorm, "Mantieni chiusa", "", "Apri quando serve");

	setBtnStatus3Way(ui.pbTrasfAccumulo, s().xTrasfDaAccumuloInCorso, s().xTrasfVersoAccumuloInCorso, bcNorm, "DA\nAccumulo", "VERSO\nAccumulo", "DA\nAccumulo", "VERSO\nAccumulo", "Accumulo\nOFF", !manuale && s().xDisabilitaAccumulo, "BLOCCO\nAccumulo");

	static bool ultimoResistenzeInUso = false;
	if (ui.pbRiscResistenze->isChecked() && ultimoResistenzeInUso && !s().xResistenzeInUso) {
		// fine ciclo: reset bottone automatico
		ui.pbRiscResistenze->setChecked(false);
	}
	ultimoResistenzeInUso = s().xResistenzeInUso;

	static bool ultimoTrasfAccumulo = false;
	if (s().xTrasfDaAccumuloInCorso || s().xTrasfVersoAccumuloInCorso) {
		if (ui.pbTrasfAccumulo->isChecked() && ui.pbRiscManuale->isChecked())
			ultimoTrasfAccumulo = true;
	} else {
		if (ultimoTrasfAccumulo) {
			s().xTrasfDaAccumulo(O_UI_CTRL) = false;
			s().xTrasfVersoAccumulo(O_UI_CTRL) = false;
		}
		ultimoTrasfAccumulo = false;
	}
}

void ControlDlg::on_pbRiscManuale_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (checked) {
		s().xSetManuale(O_UI_CTRL) = true;
		/* recover from current state */
		ui.pbRiscGas->setChecked(s().xUsaGas(O_UI_CTRL) = s().xGasInUso);
		ui.pbRiscPompaCalore->setChecked(s().xUsaPompaCalore(O_UI_CTRL) = (s().xPompaCaloreRiscInUso || s().xPompaCaloreCondInUso));
		ui.pbRiscResistenze->setChecked(s().xUsaResistenze(O_UI_CTRL) = s().xResistenzeInUso);
		s().xTrasfDaAccumulo(O_UI_CTRL) = s().xTrasfDaAccumuloInCorso;
		s().xTrasfVersoAccumulo(O_UI_CTRL) = s().xTrasfVersoAccumuloInCorso;
		ui.pbTrasfAccumulo->setChecked(s().xTrasfDaAccumulo || s().xTrasfVersoAccumulo);
	} else {
		s().xSetManuale(O_UI_CTRL) = false;
		ui.pbRiscGas->setChecked(s().xDisabilitaGas);
		ui.pbRiscPompaCalore->setChecked(s().xDisabilitaPompaCalore);
		ui.pbRiscResistenze->setChecked(s().xDisabilitaResistenze);
		ui.pbTrasfAccumulo->setChecked(s().xDisabilitaAccumulo);
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbRiscGas_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		s().xUsaGas(O_UI_CTRL) = checked;
	} else {
		s().xDisabilitaGas(O_UI_CTRL) = checked;
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbRiscPompaCalore_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		s().xUsaPompaCalore(O_UI_CTRL) = checked;
	} else {
		s().xDisabilitaPompaCalore(O_UI_CTRL) = checked;
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbRiscResistenze_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		s().xUsaResistenze(O_UI_CTRL) = checked;
	} else {
		s().xDisabilitaResistenze(O_UI_CTRL) = checked;
	}
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::on_pbTrasfAccumulo_clicked()
{
	resetCloseTimer();

	lockMutex();
	if (ui.pbRiscManuale->isChecked()) {
		if (!s().xTrasfDaAccumulo && !s().xTrasfVersoAccumulo) {
			// fermo->da
			s().xTrasfDaAccumulo(O_UI_CTRL) = true;
			s().xTrasfVersoAccumulo(O_UI_CTRL) = false;
			ui.pbTrasfAccumulo->setChecked(true);
		} else if (s().xTrasfDaAccumulo && !s().xTrasfVersoAccumulo) {
			// da->verso
			s().xTrasfDaAccumulo(O_UI_CTRL) = false;
			s().xTrasfVersoAccumulo(O_UI_CTRL) = true;
			ui.pbTrasfAccumulo->setChecked(true);
		} else if (!s().xTrasfDaAccumulo && s().xTrasfVersoAccumulo) {
			// verso->off
			s().xTrasfDaAccumulo(O_UI_CTRL) = false;
			s().xTrasfVersoAccumulo(O_UI_CTRL) = false;
			ui.pbTrasfAccumulo->setChecked(false);
		}
	} else {
		s().xDisabilitaAccumulo(O_UI_CTRL) = ui.pbTrasfAccumulo->isChecked();
	}
	unlockMutex();
}

void ControlDlg::on_pbVelMinus_clicked()
{
	resetCloseTimer();

	lockMutex();
	s().wVelFanCoil(O_UI_CTRL) -= 5;
	unlockMutex();

	ui.tlVelFanCoil->setText(s().wVelFanCoil.format());
}

void ControlDlg::on_pbVelPlus_clicked()
{
	resetCloseTimer();

	lockMutex();
	s().wVelFanCoil(O_UI_CTRL) += 5;
	unlockMutex();

	ui.tlVelFanCoil->setText(s().wVelFanCoil.format());
}

void ControlDlg::on_pbApriPlus_clicked()
{
	resetCloseTimer();

	lockMutex();
	s().wApriCucinaPerc(O_UI_CTRL) += 10;
	unlockMutex();

	ui.tlApriCucinaPerc->setText(s().wApriCucinaPerc.format());
}

void ControlDlg::on_pbApriMinus_clicked()
{
	resetCloseTimer();

	lockMutex();
	s().wApriCucinaPerc(O_UI_CTRL) -= 10;
	unlockMutex();

	ui.tlApriCucinaPerc->setText(s().wApriCucinaPerc.format());
}

void ControlDlg::on_pbForzaChiudi_toggled(bool checked)
{
	resetCloseTimer();

	lockMutex();
	s().xForzaChiudi(O_UI_CTRL) = checked;
	updateBtnStatus();
	unlockMutex();
}

void ControlDlg::resetCloseTimer()
{
	m_closeTimer.start(60000);
}

void ControlDlg::showEvent(QShowEvent *)
{
	resetCloseTimer();
}

void ControlDlg::on_pbOK_clicked()
{
	hide();
}

void ControlDlg::updateScreen()
{
	lockMutex();

	static const QTime MIDNIGHT = QTime(0, 0, 10);
	if (QTime::currentTime() < MIDNIGHT) {
		// operations to be performed every midnight
		if (ui.pbRiscManuale->isChecked())
			ui.pbRiscManuale->setChecked(false);
	}

	updateBtnStatus();

	unlockMutex();
}
