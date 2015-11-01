#include "dlg_status.h"

#include "control.h"
#include "hardware.h"

#include <QtGui>
#include <QApplication>

StatusDlg::StatusDlg(QWidget *parent, QWidget *controlDlg)
    : QWidget(parent)
    , m_controlDlg(controlDlg)
{
	ui.setupUi(this);

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
	sprintf(buf, "%.1f", control().wTemperaturaACS/10.0);
	ui.tlTempAcquaTop->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaBoiler/10.0);
	ui.tlTempAcquaBot->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaAccumulo/10.0);
	ui.tlTempAccumulo->setText(buf);
	sprintf(buf, "%.1f", control().wTemperaturaPannelli/10.0);
	ui.tlTempPannelli->setText(buf);

	sprintf(buf, "%.1f", control().wTempSoffitta/10.0);
	ui.tlTempSoffitta->setText(buf);
	sprintf(buf, "%.1f", control().wUmidSoffitta/10.0);
	ui.tlUmidSoffitta->setText(buf);

	sprintf(buf, "%i", control().wPotProdotta);
	ui.tlPotProdotta->setText(buf);
	sprintf(buf, "%i", control().wPotConsumata);
	ui.tlPotConsumata->setText(buf);

	sprintf(buf, "%.3f Wh oggi", control().wEnergProdotta/1000.0);
	ui.tlEnergiaProdotta->setText(buf);
	sprintf(buf, "%.3f Wh oggi", control().wEnergConsumata/1000.0);
	ui.tlEnergiaConsumata->setText(buf);
	if (control().tTempoAttivo.hour()) {
		sprintf(buf, "%ih%02i in attivo", control().tTempoAttivo.hour(), control().tTempoAttivo.minute());
		ui.tlBilAttivo->setText(buf);
	} else {
		sprintf(buf, "%imin in attivo", control().tTempoAttivo.minute());
		ui.tlBilAttivo->setText(buf);
	}
	sprintf(buf, "%.3f effettivi", control().wEnergPassivo/1000.0);
	ui.tlEnergPassivo->setText(buf);

	if (control().xCaldaiaInUso) {
		setLabelActive(ui.tlStatoCaldaia, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoCaldaia);
	}

	if (control().xPompaCaloreInUso) {
		setLabelActive(ui.tlStatoPompaCalore, QColor(64, 255, 64));
	} else {
		setLabelInactive(ui.tlStatoPompaCalore);
	}

	if (control().xResistenzeInUso) {
		setLabelActive(ui.tlStatoResistenze, QColor(255, 64, 64));
	} else {
		setLabelInactive(ui.tlStatoResistenze);
	}

	if (control().xFanCoil) {
		setLabelActive(ui.tlStatoVentilatore, QColor(128, 128, 255));
	} else {
		setLabelInactive(ui.tlStatoVentilatore);
	}
}
