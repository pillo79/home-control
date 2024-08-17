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

	ui.pbForzaChiudi->setChecked(s().xForzaChiudi);

	updateBtnStatus();
}

void ControlDlg::on_pbModoRisc_clicked()
{
	bool checked = ui.pbModoRisc->isChecked();

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

	QMutexLocker lock(&s().fieldLock);

	s().xModoRiscaldamento(O_UI_CTRL) = checked;
	if (!checked) {
		ui.pbGiorno->setChecked(s().xAttivaZonaGiorno(O_UI_CTRL) = false);
		ui.pbNotte->setChecked(s().xAttivaZonaNotte(O_UI_CTRL) = false);
	}
	updateBtnStatus();
}

void ControlDlg::on_pbNotte_clicked()
{
	bool checked = ui.pbNotte->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().xAttivaZonaNotte(O_UI_CTRL) = checked;
	updateBtnStatus();
}

void ControlDlg::on_pbGiorno_clicked()
{
	bool checked = ui.pbGiorno->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().xAttivaZonaGiorno(O_UI_CTRL) = checked;
	updateBtnStatus();
}

void ControlDlg::on_pbSoffitta_clicked()
{
	bool checked = ui.pbSoffitta->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().xAttivaZonaSoffitta(O_UI_CTRL) = checked;
	updateBtnStatus();
}

void ControlDlg::on_pbFanCoil_clicked()
{
	bool checked = ui.pbFanCoil->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().xAttivaFanCoil(O_UI_CTRL) = checked;
	updateBtnStatus();

	if (!checked && ui.pbForzaChiudi->isChecked())
		ui.pbForzaChiudi->setChecked(false);
}

void ControlDlg::on_pbProg_clicked()
{
	bool checked = ui.pbProg->isChecked();

	QMutexLocker lock(&s().fieldLock);

	s().xAttivaProg(O_UI_CTRL) = checked;
	updateBtnStatus();
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

	setBtnStatus(ui.pbForzaChiudi, false, bcNorm, "Mantieni chiusa", "", "Apri quando serve");

	setBtnStatus3Way(ui.pbTrasfAccumulo, s().xTrasfDaAccumuloInCorso, s().xTrasfVersoAccumuloInCorso, bcNorm, "Accumulo\nATTIVO", "Accumulo\nATTIVO", "DA\nAccumulo", "VERSO\nAccumulo", "Accumulo\nOFF", !manuale && s().xDisabilitaAccumulo, "BLOCCO\nAccumulo");
}

void ControlDlg::on_pbRiscManuale_clicked()
{
	bool checked = ui.pbRiscManuale->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	if (checked) {
		s().xSetManuale(O_UI_CTRL) = true;
		/* recover from current state */
		ui.pbRiscGas->setChecked(s().xUsaGas(O_UI_CTRL) = s().xGasInUso);
		ui.pbRiscPompaCalore->setChecked(s().xUsaPompaCalore(O_UI_CTRL) = (s().xPompaCaloreRiscInUso || s().xPompaCaloreCondInUso));
		s().xTrasfAccumulo(O_UI_CTRL) = s().xTrasfDaAccumuloInCorso || s().xTrasfVersoAccumuloInCorso;
		ui.pbTrasfAccumulo->setChecked(s().xTrasfAccumulo);
	} else {
		s().xSetManuale(O_UI_CTRL) = false;
		ui.pbRiscGas->setChecked(s().xDisabilitaGas);
		ui.pbRiscPompaCalore->setChecked(s().xDisabilitaPompaCalore);
		ui.pbTrasfAccumulo->setChecked(s().xDisabilitaAccumulo);
	}
	updateBtnStatus();
}

void ControlDlg::on_pbRiscGas_clicked()
{
	bool checked = ui.pbRiscGas->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	if (ui.pbRiscManuale->isChecked()) {
		s().xUsaGas(O_UI_CTRL) = checked;
	} else {
		s().xDisabilitaGas(O_UI_CTRL) = checked;
	}
	updateBtnStatus();
}

void ControlDlg::on_pbRiscPompaCalore_clicked()
{
	bool checked = ui.pbRiscPompaCalore->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	if (ui.pbRiscManuale->isChecked()) {
		s().xUsaPompaCalore(O_UI_CTRL) = checked;
	} else {
		s().xDisabilitaPompaCalore(O_UI_CTRL) = checked;
	}
	updateBtnStatus();
}

void ControlDlg::on_pbTrasfAccumulo_clicked()
{
	bool checked = ui.pbTrasfAccumulo->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	if (ui.pbRiscManuale->isChecked()) {
		s().xTrasfAccumulo(O_UI_CTRL) = checked;
	} else {
		s().xDisabilitaAccumulo(O_UI_CTRL) = checked;
	}
	updateBtnStatus();
}

void ControlDlg::on_pbVelMinus_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().wVelFanCoil(O_UI_CTRL) -= 5;

	ui.tlVelFanCoil->setText(s().wVelFanCoil.format());
}

void ControlDlg::on_pbVelPlus_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().wVelFanCoil(O_UI_CTRL) += 5;

	ui.tlVelFanCoil->setText(s().wVelFanCoil.format());
}

void ControlDlg::on_pbApriPlus_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().wApriCucinaPerc(O_UI_CTRL) += 10;

	ui.tlApriCucinaPerc->setText(s().wApriCucinaPerc.format());
}

void ControlDlg::on_pbApriMinus_clicked()
{
	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().wApriCucinaPerc(O_UI_CTRL) -= 10;

	ui.tlApriCucinaPerc->setText(s().wApriCucinaPerc.format());
}

void ControlDlg::on_pbForzaChiudi_clicked()
{
	bool checked = ui.pbForzaChiudi->isChecked();

	resetCloseTimer();

	QMutexLocker lock(&s().fieldLock);

	s().xForzaChiudi(O_UI_CTRL) = checked;
	updateBtnStatus();
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
	static const QTime MIDNIGHT = QTime(0, 0, 10);
	if (QTime::currentTime() < MIDNIGHT) {
		// operations to be performed every midnight
		if (ui.pbRiscManuale->isChecked())
			ui.pbRiscManuale->setChecked(false);
	}

	updateBtnStatus();
}
