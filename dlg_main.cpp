#include <QtGui>
#include "dlg_main.h"

#include "control.h"

MainDlg::MainDlg(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
}

void MainDlg::on_pbNotte_toggled(bool checked)
{
	QMutexLocker lock(&control().mFields);
	control().xRiscaldaNotte = checked;
}

void MainDlg::on_pbGiorno_toggled(bool checked)
{
	QMutexLocker lock(&control().mFields);
	control().xRiscaldaGiorno = checked;
}

void MainDlg::on_pbSoffitta_toggled(bool checked)
{
	QMutexLocker lock(&control().mFields);
	control().xRiscaldaSoffitta = checked;
}

void MainDlg::on_pbFanCoil_toggled(bool checked)
{
	QMutexLocker lock(&control().mFields);
	control().xFanCoil = checked;
}
