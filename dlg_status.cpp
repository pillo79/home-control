#include "dlg_status.h"

#include "control.h"
#include "hardware.h"
#include "dlg_alarm.h"

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

	ui.tlPotProdotta->setValue(&control().wPotProdotta,		QColor(64,192,255), QColor(255,192,64));
	ui.tlPotConsumata->setValue(&control().wPotConsumata,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAcquaTop->setValue(&control().wTemperaturaACS,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAcquaBot->setValue(&control().wTemperaturaBoiler,	QColor(64,192,255), QColor(255,192,64));
	ui.tlTempCantinaTop->setValue(&control().wTempLegnaH,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempCantinaBot->setValue(&control().wTempLegnaL,		QColor(64,192,255), QColor(255,192,64));
	ui.tlTempPannelli->setValue(&control().wTemperaturaPannelli,	QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAccumulo->setValue(&control().wTemperaturaAccumulo,	QColor(64,192,255), QColor(255,192,64));
	ui.tlTempEsterno->setValue(&control().wTempEsterno,		QColor(64,192,255), QColor(255,192,64));

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);
}

void StatusDlg::on_pbSetNotte_toggled(bool)
{
	m_controlDlg->show();
}

void StatusDlg::on_pbSetGiorno_toggled(bool)
{
	m_controlDlg->show();
}

void StatusDlg::on_pbSetSoffitta_toggled(bool)
{
	m_controlDlg->show();
}

void StatusDlg::updateStatoRisc()
{
/*	if (ui.pbRiscGas->isChecked()) {
		ui.pbRiscGas->setPalette(QPalette(QColor(255, 64, 64)));
		ui.pbRiscGas->setText("Gas ON");
	} else if (control().xGasInUso) {
		ui.pbRiscGas->setPalette(QPalette(QColor(220,220,128)));
		ui.pbRiscGas->setText("Gas auto ON");
	} else {
		ui.pbRiscGas->setPalette(QApplication::palette());
		ui.pbRiscGas->setText("Gas OFF");
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
*/
}

static void setButtonActive(QPushButton *button, QString text, QColor color)
{
	QPalette palette = button->palette();
	palette.setColor(button->backgroundRole(), color);
	button->setPalette(palette);
	button->setText(text);
}

static void setButtonInactive(QPushButton *button, QString text)
{
	button->setPalette(QApplication::palette());
	button->setText(text);
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

	QMutexLocker lock(&control().mFields);

	ui.tlTempAcquaTop->setText(control().wTemperaturaACS.format());
	ui.tlTempAcquaBot->setText(control().wTemperaturaBoiler.format());
	ui.tlTempCantinaTop->setText(control().wTempLegnaH.format());
	ui.tlTempCantinaBot->setText(control().wTempLegnaL.format());
	ui.tlTempAccumulo->setText(control().wTemperaturaAccumulo.format());
	ui.tlTempPannelli->setText(control().wTemperaturaPannelli.format());

//	ui.tlTempSoffitta->setText(control().wTempSoffitta.format());
//	ui.tlUmidSoffitta->setText(control().wUmidSoffitta.format());

	ui.tlTempEsterno->setText(control().wTempEsterno.format());

	ui.tlPotProdotta->setText(control().wPotProdotta.format());
	ui.tlPotConsumata->setText(control().wPotConsumata.format());

	sprintf(buf, "%.3f kWh oggi", control().wEnergProdotta.value());
	ui.tlEnergiaProdotta->setText(buf);
	sprintf(buf, "%.3f kWh oggi", control().wEnergConsumata.value());
	ui.tlEnergiaConsumata->setText(buf);
	if (control().tTempoAttivo.hour()) {
		sprintf(buf, "%ih%02i in attivo", control().tTempoAttivo.hour(), control().tTempoAttivo.minute());
		ui.tlBilAttivo->setText(buf);
	} else {
		sprintf(buf, "%imin in attivo", control().tTempoAttivo.minute());
		ui.tlBilAttivo->setText(buf);
	}
	sprintf(buf, "%.3f effettivi", control().wEnergPassivo.value());
	ui.tlEnergPassivo->setText(buf);

	if (control().xGasInUso) {
		setLabelActive(ui.tlStatoGas, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoGas);
	}

	if (control().xLegnaInUso) {
		setLabelActive(ui.tlStatoLegna, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoLegna);
	}

	if (control().xPompaCaloreRiscInUso) {
		setLabelActive(ui.tlStatoPompaCalore, QColor(255,192,64));
	} else if (control().xPompaCaloreCondInUso) {
		setLabelActive(ui.tlStatoPompaCalore, QColor(64,192,255));
	} else {
		setLabelInactive(ui.tlStatoPompaCalore);
	}

	if (control().xResistenzeInUso) {
		setLabelActive(ui.tlStatoResistenze, QColor(255, 64, 64));
		sprintf(buf, "<b>Resistenze</b><br/>%.0f W", control().wPotResistenze.value());
		ui.tlStatoResistenze->setText(buf);
	} else {
		setLabelInactive(ui.tlStatoResistenze);
		ui.tlStatoResistenze->setText("<b>Resistenze</b>");
	}

	if (control().xTrasfDaAccumuloInCorso) {
		ui.groupBox_6->setTitle("DA Accumulo");
	} else if (control().xTrasfVersoAccumuloInCorso) {
		ui.groupBox_6->setTitle("VERSO Accumulo");
	} else
		ui.groupBox_6->setTitle("Accumulo");

	if (control().xAttivaFanCoil) {
		if (control().xPompaCaloreCondInUso)
			setLabelActive(ui.tlStatoVentilatore, QColor(64,192,255));
		else
			setLabelActive(ui.tlStatoVentilatore, QColor(255,192,64));
	} else {
		setLabelInactive(ui.tlStatoVentilatore);
	}

	if (control().xPompaCaloreCondInUso && control().xAttivaZonaSoffitta) {
		setLabelActive(ui.tlStatoRadiatori, QColor(64,192,255));
	} else if (control().xImpiantoAttivo && (control().xAttivaZonaNotte || control().xAttivaZonaGiorno || control().xAttivaZonaSoffitta)) {
		setLabelActive(ui.tlStatoRadiatori, QColor(255, 192, 64));
	} else {
		setLabelInactive(ui.tlStatoRadiatori);
	}

	QColor onColor;
	QColor autoColor = QColor(255, 255, 128);
	QString onString;
	if (control().xModoRiscaldamento) {
		onColor = QColor(255, 64, 64);
		onString = "caldo";
		if (control().xAttivaZonaNotte)
			setButtonActive(ui.pbSetNotte, onString, control().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetNotte, "spento");
		if (control().xAttivaZonaGiorno)
			setButtonActive(ui.pbSetGiorno, onString, control().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetGiorno, "spento");
		if (control().xAttivaZonaSoffitta)
			setButtonActive(ui.pbSetSoffitta, onString, control().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetSoffitta, "spento");
	} else {
		onColor = QColor(64, 192, 255);
		onString = "freddo";
		if (control().xPompaCaloreCondInUso && control().xAttivaFanCoil) {
			setButtonActive(ui.pbSetNotte, onString, control().xImpiantoAttivo? onColor : autoColor);
			setButtonActive(ui.pbSetGiorno, onString, control().xImpiantoAttivo? onColor : autoColor);
		} else {
			setButtonInactive(ui.pbSetNotte, "spento");
			setButtonInactive(ui.pbSetGiorno, "spento");
		}
		if (control().xAttivaZonaSoffitta)
			setButtonActive(ui.pbSetSoffitta, onString, control().xImpiantoAttivo? onColor : autoColor);
		else
			setButtonInactive(ui.pbSetSoffitta, "spento");
	}

	if ((control().wCommErrorMask != 0) && (QTime::currentTime().msec() < 500))
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
					.arg(control().wResetPLCs);
	if (control().wResetPLCs) {
		status += QString(", ultimo il %5 alle %6")
					.arg(control().dtLastResetPLC.toString("d/MM"))
					.arg(control().dtLastResetPLC.toString("h:mm"));
	}
	ui.tlSystemStatus->setText(status);
}
