#ifndef __DLG_STATUS_H__
#define __DLG_STATUS_H__

#include <QWidget>
#include <QTimer>

#include "ui_dlg_status.h"

class StatusDlg : public QWidget
{
	Q_OBJECT

	public:
		StatusDlg(QWidget *parent = 0, QWidget *controlDlg = 0);

	public slots:
		void on_pbSetNotte_clicked();
		void on_pbSetGiorno_clicked();
		void on_pbSetSoffitta_clicked();

		void on_pbSetProg_clicked();
		void on_pbConfig_clicked();

		void updateScreen();

	private:
		Ui::StatusDlg ui;
		QWidget *m_controlDlg;
		QWidget *m_alarmDlg;
};

#endif /* __DLG_STATUS_H__ */
