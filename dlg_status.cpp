#include "dlg_status.h"

#include "control.h"
#include "hardware.h"
#include "state.h"
#include "dlg_alarm.h"

#include "ctrlvalue.h"
#include "ctrlobserver.h"

#include <QtGui>
#include <QApplication>

const static QDateTime startup = QDateTime::currentDateTime();

StatusDlg::StatusDlg(QWidget *parent, QWidget *controlDlg)
    : QWidget(parent)
    , m_controlDlg(controlDlg)
{
	ui.setupUi(this);

	m_alarmDlg = new AlarmDlg(this);
	m_alarmDlg->move(width()-m_alarmDlg->width(), height()-m_alarmDlg->height());
	m_alarmDlg->hide();

	ui.tlPotProdotta->setValue(&s().wPotProdotta,		QColor(64,192,255), QColor(255,192,64));
	ui.tlPotConsumata->setValue(&s().wPotConsumata,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAcquaTop->setValue(&s().wTemperaturaACS,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAcquaBot->setValue(&s().wTemperaturaBoiler,	QColor(64,192,255), QColor(255,192,64));
	ui.tlTempCantinaTop->setValue(&s().wTempLegnaH,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempCantinaBot->setValue(&s().wTempLegnaL,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempPannelli->setValue(&s().wTemperaturaPannelli,	QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAccumulo->setValue(&s().wTemperaturaAccumulo,	QColor(64,192,255), QColor(255,192,64));
	ui.tlTempEsterno->setValue(&s().wTempEsterno,		QColor(64,192,255), QColor(255,192,64));

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);
}

void StatusDlg::on_pbSetNotte_clicked()
{
	QMutexLocker lock(&s().fieldLock);

	if (s().xModoRiscaldamento) {
		s().xAttivaZonaNotte(O_UI_STAT) = ui.pbSetNotte->isChecked();
	} else {
		s().xAttivaFanCoil(O_UI_STAT) = ui.pbSetNotte->isChecked();
	}
}

void StatusDlg::on_pbSetGiorno_clicked()
{
	QMutexLocker lock(&s().fieldLock);

	if (s().xModoRiscaldamento) {
		s().xAttivaZonaGiorno(O_UI_STAT) = ui.pbSetGiorno->isChecked();
	} else {
		s().xAttivaFanCoil(O_UI_STAT) = ui.pbSetGiorno->isChecked();
	}
}

void StatusDlg::on_pbSetSoffitta_clicked()
{
	QMutexLocker lock(&s().fieldLock);

	s().xAttivaZonaSoffitta(O_UI_STAT) = ui.pbSetSoffitta->isChecked();
}

void StatusDlg::on_pbConfig_clicked()
{
	m_controlDlg->show();
}

void StatusDlg::on_pbSetProg_clicked()
{
	QMutexLocker lock(&s().fieldLock);

	s().xAttivaProg(O_UI_STAT) = ui.pbSetProg->isChecked();
}

static void setButtonActive(QPushButton *button, QString text, QColor color)
{
	QPalette palette = button->palette();
	palette.setColor(button->backgroundRole(), color);
	button->setPalette(palette);
	button->setText(text);
	button->setChecked(true);
}

static void setButtonInactive(QPushButton *button, QString text)
{
	button->setPalette(QApplication::palette());
	button->setText(text);
	button->setChecked(false);
}

static void setLabelActive(QLabel *label, QColor color)
{
	QPalette palette = label->palette();
	palette.setColor(label->backgroundRole(), color);
	label->setPalette(palette);
	label->setFrameShape(QFrame::Box);
	label->setEnabled(true);
}

static void setLabelInactive(QLabel *label)
{
	label->setEnabled(false);
	label->setPalette(QApplication::palette());
	label->setFrameShape(QFrame::NoFrame);
}

void StatusDlg::updateScreen()
{
	char buf[256];

	ui.tlData->setText(QDate::currentDate().toString("dd/MM/yyyy"));
	ui.tlOra->setText(QTime::currentTime().toString("h:mm"));

	ui.tlTempAcquaTop->setText(s().wTemperaturaACS.format());
	ui.tlTempAcquaBot->setText(s().wTemperaturaBoiler.format());
	ui.tlTempCantinaTop->setText(s().wTempLegnaH.format());
	ui.tlTempCantinaBot->setText(s().wTempLegnaL.format());
	ui.tlTempAccumulo->setText(s().wTemperaturaAccumulo.format());
	ui.tlTempPannelli->setText(s().wTemperaturaPannelli.format());

	ui.tlTempEsterno->setText(s().wTempEsterno.format());

	ui.tlPotProdotta->setText(s().wPotProdotta.format());
	ui.tlPotConsumata->setText(s().wPotConsumata.format());

	sprintf(buf, "%.3f kWh oggi", s().wEnergProdotta.value());
	ui.tlEnergiaProdotta->setText(buf);
	sprintf(buf, "%.3f kWh oggi", s().wEnergConsumata.value());
	ui.tlEnergiaConsumata->setText(buf);
	if (s().tTempoAttivo.hour()) {
		sprintf(buf, "%ih%02i in attivo", s().tTempoAttivo.hour(), s().tTempoAttivo.minute());
		ui.tlBilAttivo->setText(buf);
	} else {
		sprintf(buf, "%imin in attivo", s().tTempoAttivo.minute());
		ui.tlBilAttivo->setText(buf);
	}
	sprintf(buf, "%.3f effettivi", s().wEnergPassivo.value());
	ui.tlEnergPassivo->setText(buf);

	if (s().xGasInUso) {
		setLabelActive(ui.tlStatoGas, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoGas);
	}

	if (s().xLegnaInUso) {
		setLabelActive(ui.tlStatoLegna, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoLegna);
	}

	if (s().xPompaCaloreRiscInUso) {
		setLabelActive(ui.tlStatoPompaCalore, QColor(255,192,64));
	} else if (s().xPompaCaloreCondInUso) {
		setLabelActive(ui.tlStatoPompaCalore, QColor(64,192,255));
	} else {
		setLabelInactive(ui.tlStatoPompaCalore);
	}

	if (s().xTrasfDaAccumuloInCorso) {
		ui.groupBox_6->setTitle("DA Accumulo");
	} else if (s().xTrasfVersoAccumuloInCorso) {
		ui.groupBox_6->setTitle("VERSO Accumulo");
	} else
		ui.groupBox_6->setTitle("Accumulo");

	if (s().xAttivaFanCoil) {
		if (s().xPompaCaloreCondInUso)
			setLabelActive(ui.tlStatoVentilatore, QColor(64,192,255));
		else
			setLabelActive(ui.tlStatoVentilatore, QColor(255,192,64));
	} else {
		setLabelInactive(ui.tlStatoVentilatore);
	}

	if (s().xPompaCaloreCondInUso && s().xAttivaZonaSoffitta) {
		setLabelActive(ui.tlStatoRadiatori, QColor(64,192,255));
	} else if (s().xImpiantoAttivo && (s().xAttivaZonaNotte || s().xAttivaZonaGiorno || s().xAttivaZonaSoffitta)) {
		setLabelActive(ui.tlStatoRadiatori, QColor(255, 192, 64));
	} else {
		setLabelInactive(ui.tlStatoRadiatori);
	}

	QColor onColor;
	QColor autoColor = QColor(255, 255, 128);
	QString onString;
	if (s().xModoRiscaldamento) {
		onColor = QColor(255, 64, 64);
		onString = "caldo";
		if (s().xAttivaZonaNotte)
			setButtonActive(ui.pbSetNotte, onString, s().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetNotte, "spento");
		if (s().xAttivaZonaGiorno)
			setButtonActive(ui.pbSetGiorno, onString, s().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetGiorno, "spento");
		if (s().xAttivaZonaSoffitta)
			setButtonActive(ui.pbSetSoffitta, onString, s().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetSoffitta, "spento");
	} else {
		onColor = QColor(64, 192, 255);
		onString = "freddo";
		if (s().xPompaCaloreCondInUso && s().xAttivaFanCoil) {
			setButtonActive(ui.pbSetNotte, onString, s().xImpiantoAttivo? onColor : autoColor);
			setButtonActive(ui.pbSetGiorno, onString, s().xImpiantoAttivo? onColor : autoColor);
		} else {
			setButtonInactive(ui.pbSetNotte, "spento");
			setButtonInactive(ui.pbSetGiorno, "spento");
		}
		if (s().xAttivaZonaSoffitta)
			setButtonActive(ui.pbSetSoffitta, onString, s().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetSoffitta, "spento");
	}

	if (s().xAttivaProg)
		setButtonActive(ui.pbSetProg, "", s().xImpiantoAttivo? onColor : autoColor);
	else
		setButtonInactive(ui.pbSetProg, "");

	if ((s().wCommErrorMask != 0) && (QTime::currentTime().msec() < 500))
		m_alarmDlg->show();
	else
		m_alarmDlg->hide();

	QString status;

	quint32 elapsed  = startup.secsTo(QDateTime::currentDateTime())/60;
	quint32 up_mins  = elapsed % 60; elapsed /= 60;
	quint32 up_hours = elapsed % 24; elapsed /= 24;
	quint32 up_days  = elapsed;
	status += QString("Attivo da %1g %2h %3m, %4 reset")
					.arg(up_days)
					.arg(up_hours)
					.arg(up_mins)
					.arg(s().wResetPLCs);
	if (s().wResetPLCs) {
		status += QString(", ultimo il %5 alle %6")
					.arg(s().dtLastResetPLC.toString("d/MM"))
					.arg(s().dtLastResetPLC.toString("h:mm"));
	}
	ui.tlSystemStatus->setText(status);
}
