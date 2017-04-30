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

	ui.tlPotProdotta->setValue(&control().wPotProdotta, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlPotConsumata->setValue(&control().wPotConsumata, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAcquaTop->setValue(&control().wTemperaturaACS, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAcquaBot->setValue(&control().wTemperaturaBoiler, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempPannelli->setValue(&control().wTemperaturaPannelli, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempAccumulo->setValue(&control().wTemperaturaAccumulo, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempGiorno->setValue(&control().wTempGiorno, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlUmidGiorno->setValue(&control().wUmidGiorno, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempNotte->setValue(&control().wTempNotte, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlUmidNotte->setValue(&control().wUmidNotte, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempSoffitta->setValue(&control().wTempSoffitta, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlUmidSoffitta->setValue(&control().wUmidSoffitta, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlTempEsterno->setValue(&control().wTempEsterno, "", QColor(64,192,255), QColor(255,192,64));
	ui.tlUmidEsterno->setValue(&control().wUmidEsterno, "", QColor(64,192,255), QColor(255,192,64));

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);
}

void StatusDlg::on_pbSetNotte_toggled(bool checked)
{
	m_controlDlg->show();
}

void StatusDlg::on_pbSetGiorno_toggled(bool checked)
{
	m_controlDlg->show();
}

void StatusDlg::on_pbSetSoffitta_toggled(bool checked)
{
	m_controlDlg->show();
}

void StatusDlg::updateStatoRisc()
{
/*	if (ui.pbRiscCaldaia->isChecked()) {
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
	sprintf(buf, "%.1f", control().wTemperaturaACS.value());
	ui.tlTempAcquaTop->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaBoiler.value());
	ui.tlTempAcquaBot->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaAccumulo.value());
	ui.tlTempAccumulo->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaPannelli.value());
	ui.tlTempPannelli->setText(buf);

	sprintf(buf, "%.1f", control().wTempSoffitta.value());
	ui.tlTempSoffitta->setText(buf);
	sprintf(buf, "%.1f", control().wUmidSoffitta.value());
	ui.tlUmidSoffitta->setText(buf);

	sprintf(buf, "%.1f", control().wTempEsterno.value());
	ui.tlTempEsterno->setText(buf);

	sprintf(buf, "%.0f", control().wPotProdotta.value());
	ui.tlPotProdotta->setText(buf);
	sprintf(buf, "%.0f", control().wPotConsumata.value());
	ui.tlPotConsumata->setText(buf);

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

	if (control().xCaldaiaInUso) {
		setLabelActive(ui.tlStatoCaldaia, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoCaldaia);
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
	} else {
		setLabelInactive(ui.tlStatoResistenze);
	}

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
	} else if (control().xAttivaZonaNotte || control().xAttivaZonaGiorno || control().xAttivaZonaSoffitta) {
		setLabelActive(ui.tlStatoRadiatori, QColor(255, 192, 64));
	} else {
		setLabelInactive(ui.tlStatoRadiatori);
	}

	QColor onColor;
	QString onString;
	if (control().xModoRiscaldamento) {
		onColor = QColor(255, 64, 64);
		onString = "caldo";
		if (control().xAttivaZonaNotte)
			setButtonActive(ui.pbSetNotte, onString, onColor);
		else
			setButtonInactive(ui.pbSetNotte, "spento");
		if (control().xAttivaZonaGiorno)
			setButtonActive(ui.pbSetGiorno, onString, onColor);
		else
			setButtonInactive(ui.pbSetGiorno, "spento");
		if (control().xAttivaZonaSoffitta)
			setButtonActive(ui.pbSetSoffitta, onString, onColor);
		else
			setButtonInactive(ui.pbSetSoffitta, "spento");
	} else {
		onColor = QColor(64, 192, 255);
		onString = "freddo";
		if (control().xPompaCaloreCondInUso && control().xAttivaFanCoil) {
			setButtonActive(ui.pbSetNotte, onString, onColor);
			setButtonActive(ui.pbSetGiorno, onString, onColor);
		} else {
			setButtonInactive(ui.pbSetNotte, "spento");
			setButtonInactive(ui.pbSetGiorno, "spento");
		}
		if (control().xAttivaZonaSoffitta)
			setButtonActive(ui.pbSetSoffitta, onString, onColor);
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
